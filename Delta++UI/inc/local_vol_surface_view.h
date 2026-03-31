#pragma once

#include <Walnut/Layer.h>

#include "local_vol_surface_state.h"

class LocalVolSurfaceWindow : public Walnut::Layer
{
public:
    LocalVolSurfaceWindow();

    void OnUIRender() override;

private:
    DPP::LocalVolSurfaceState m_state;
    bool m_showIv3d = false;
    bool m_showLv3d = false;
    bool m_showCall3d = false;
};

