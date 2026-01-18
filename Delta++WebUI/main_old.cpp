#include <stdio.h>
#include <iostream>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Delta++ includes
#include \"Delta++/black_scholes_engine.h\"
#include \"Delta++/enums.h\"
#include \"Delta++/market.h\"
#include \"Delta++/trade.h\"
#include \"Delta++/calc.h\"

// Global variables
GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Delta++ Application State
struct DeltaUIState {
    // Trade parameters
    int exerciseTypeIdx = 0;
    int payoffTypeIdx = 0;
    double maturity = 1.0;
    double strike = 100.0;
    
    // Market parameters  
    double underlyingPrice = 100.0;
    double volatility = 0.2;
    double interestRate = 0.05;
    
    // Calculation parameters
    int calculationMethodIdx = 2; // Start with Black-Scholes
    int steps = 100;
    int sims = 10000;
    bool dynamicRecalc = true;
    
    // Results
    bool hasResults = false;
    double pv = 0.0;
    double delta = 0.0;
    double gamma = 0.0;
    double vega = 0.0;
    double rho = 0.0;
    std::string errorMessage;
    
    // UI state
    bool valueChanged = false;
    bool calculatePressed = false;
    
    // Combo box data
    const char* exerciseTypes[2] = {"European", "American"};
    const char* payoffTypes[2] = {"Call", "Put"};
    const char* calculationMethods[3] = {"Monte Carlo", "Binomial", "Black-Scholes"};
};

DeltaUIState g_state;

void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void calculatePricing() {
    try {
        // Create trade data
        DPP::OptionData trade;
        trade.m_optionExerciseType = static_cast<DPP::OptionExerciseType>(g_state.exerciseTypeIdx);
        trade.m_optionPayoffType = static_cast<DPP::OptionPayoffType>(g_state.payoffTypeIdx);
        trade.m_maturity = g_state.maturity;
        trade.m_strike = g_state.strike;
        
        // Create market data
        DPP::MarketData market;
        market.m_underlyingPrice = g_state.underlyingPrice;
        market.m_vol = g_state.volatility;
        market.m_interestRate = g_state.interestRate;
        
        // For now, only implement Black-Scholes
        if (g_state.calculationMethodIdx == 2) { // Black-Scholes
            DPP::BlackScholesEngine engine;
            
            auto result = engine.calculatePV(trade, market);
            if (result.has_value()) {
                g_state.pv = result.value();
                g_state.hasResults = true;
                g_state.errorMessage.clear();
                
                // Calculate Greeks
                auto deltaResult = engine.calculateDelta(trade, market);
                if (deltaResult.has_value()) g_state.delta = deltaResult.value();
                
                auto gammaResult = engine.calculateGamma(trade, market);
                if (gammaResult.has_value()) g_state.gamma = gammaResult.value();
                
                auto vegaResult = engine.calculateVega(trade, market);
                if (vegaResult.has_value()) g_state.vega = vegaResult.value();
                
                auto rhoResult = engine.calculateRho(trade, market);
                if (rhoResult.has_value()) g_state.rho = rhoResult.value();
            } else {
                g_state.errorMessage = "Failed to calculate Black-Scholes pricing";
                g_state.hasResults = false;
            }
        } else {
            g_state.errorMessage = "Only Black-Scholes engine is currently supported in WebAssembly";
            g_state.hasResults = false;
        }
    } catch (const std::exception& e) {
        g_state.errorMessage = std::string("Calculation error: ") + e.what();
        g_state.hasResults = false;
    }
}

void renderTradeParams() {
    if (ImGui::CollapsingHeader("Trade", ImGuiTreeNodeFlags_DefaultOpen)) {
        g_state.valueChanged |= ImGui::Combo("Exercise", &g_state.exerciseTypeIdx, g_state.exerciseTypes, 2);
        ImGui::SameLine(); HelpMarker("Europeans exercise at maturity while Americans can exercise at any timestep");
        
        g_state.valueChanged |= ImGui::Combo("Payoff", &g_state.payoffTypeIdx, g_state.payoffTypes, 2);
        ImGui::SameLine(); HelpMarker("Calls pay off when above the strike and Puts payoff when below");
        
        g_state.valueChanged |= ImGui::InputDouble("Maturity (Y)", &g_state.maturity, 0.25, 1.0, "%.2f");
        ImGui::SameLine(); HelpMarker("Time to maturity in years");
        
        g_state.valueChanged |= ImGui::InputDouble("Strike Price", &g_state.strike, 1.0, 10.0, "%.2f");
        ImGui::SameLine(); HelpMarker("Strike Price");
    }
}

void renderMarketParams() {
    if (ImGui::CollapsingHeader("Market", ImGuiTreeNodeFlags_DefaultOpen)) {
        g_state.valueChanged |= ImGui::InputDouble("Underlying Value", &g_state.underlyingPrice, 1.0, 10.0, "%.2f");
        ImGui::SameLine(); HelpMarker("The current value of the underlying");
        
        g_state.valueChanged |= ImGui::InputDouble("Volatility", &g_state.volatility, 0.01, 0.1, "%.2f");
        ImGui::SameLine(); HelpMarker("Constant Volatility");
        
        g_state.valueChanged |= ImGui::InputDouble("Interest Rate", &g_state.interestRate, 0.01, 0.1, "%.2f");
        ImGui::SameLine(); HelpMarker("Risk-free interest rate");
    }
}

void renderCalcParams() {
    if (ImGui::CollapsingHeader("Calculation", ImGuiTreeNodeFlags_DefaultOpen)) {
        g_state.valueChanged |= ImGui::Checkbox("Dynamic Recalculation", &g_state.dynamicRecalc);
        
        g_state.valueChanged |= ImGui::Combo("Engine", &g_state.calculationMethodIdx, g_state.calculationMethods, 3);
        ImGui::SameLine(); HelpMarker("Engines determine the calculation technique");
        
        if (g_state.calculationMethodIdx != 2) { // Not Black-Scholes
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Note: Only Black-Scholes is supported in WebAssembly version");
        }
        
        if (!g_state.dynamicRecalc) {
            g_state.calculatePressed = ImGui::Button("Calculate");
        }
    }
}

void renderResults() {
    if (!g_state.errorMessage.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", g_state.errorMessage.c_str());
        return;
    }
    
    if (g_state.hasResults) {
        if (ImGui::BeginTable("Results", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV)) {
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("PV");
            ImGui::TableNextColumn(); ImGui::Text("%.4f", g_state.pv);
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("Delta");
            ImGui::TableNextColumn(); ImGui::Text("%.4f", g_state.delta);
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("Gamma");
            ImGui::TableNextColumn(); ImGui::Text("%.4f", g_state.gamma);
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("Vega");
            ImGui::TableNextColumn(); ImGui::Text("%.4f", g_state.vega);
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("Rho");
            ImGui::TableNextColumn(); ImGui::Text("%.4f", g_state.rho);
            
            ImGui::EndTable();
        }
    }
}

void loop() {
    glfwPollEvents();
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Main Delta++ UI Window
    ImGui::Begin("Delta++ WebAssembly Pricer");
    
    renderTradeParams();
    renderMarketParams();
    renderCalcParams();
    
    // Trigger calculation if needed
    if ((g_state.dynamicRecalc && g_state.valueChanged) || g_state.calculatePressed) {
        calculatePricing();
    }
    
    renderResults();
    
    // Reset change flags
    g_state.valueChanged = false;
    g_state.calculatePressed = false;
    
    ImGui::End();
    
    // Render
    ImGui::Render();
    
    int display_w, display_h;
    glfwMakeContextCurrent(g_window);
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(g_window);
}

int init_gl() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    // Create window with minimal size (will be resized automatically)
    g_window = glfwCreateWindow(1, 1, "Delta++ WebAssembly", NULL, NULL);
    if (g_window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(g_window);
    
    return 0;
}

int init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(g_window, "#canvas");
#endif
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    
    // Setup style
    ImGui::StyleColorsDark();
    
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Disable .ini file
    
    return 0;
}

int init() {
    if (init_gl() != 0) return 1;
    if (init_imgui() != 0) return 1;
    return 0;
}

void quit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

int main(int argc, char** argv) {
    if (init() != 0) return 1;
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    // For desktop testing
    while (!glfwWindowShouldClose(g_window)) {
        loop();
        glfwSwapBuffers(g_window);
    }
#endif
    
    quit();
    return 0;
}