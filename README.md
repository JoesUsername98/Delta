# Delta

This started off as a working example of Shreve Stochastic Calculus I. After about 2 years, I revisited the project and implemented some new features that I understood after reading Implementing Quant Lib, Paul Willmott 1&2 and flicking through Hull. The initail aim was to aid in basic learning of the BAPM. I later learned of DerivaGens existence... thanks Hull.

## Getting Started

I would like to think this works straight out of the box being a .Net program apart from needing the Extended.Wpf.Toolkit by Xceed for a UI element I liked. K.I.S.S, Just use Nuget for this.

## Running the tests

This is a WPF application so I expect you to use a windows based IDE. To that end, you should have a test explorer there.

## Directors Commentary

I may end up writing more characters here than in the entire of my project!
To start, I would like to say that this was designed from inception to be a showcase project with a start and finishing pont. I think I am now at version 1.0. I have a minimum viable "product". It has a UI and it does some maths. The maths was the easy part and the UI was essential to keep your ape brain stimulated. Something this project was never meant to be was perfect.

From inception of this project I had two main themes, to relegiously follow TDD and menaically implement as many design patterns as I had read about in the gang of four book, both had more or less stopped by the projects completion [for worse and better, respectively]. TDD died off because I was being lazy, honestly. To test in a professional way, I would have to revamp almost all my tests that I had written so far. I stopped using design patterns because I realised that they are a tool best used seldomly ( apart from strategy, everyone likes the strategy pattern .) This is an important part about growing up as someone who engineers software you learn that the answer is always 'it depends', and when it comes to design patterns, the answer is almost always 'nah, you ain't gunna need it.'

At work I mainly write C++, so I am starting to develop an unhealthy obsession with performance. I started testing the binomial model I had implemented for larger and larger numbers of time steps and oh boy is there something terrible wrong there. I know the path dependant method is O(2^n) but there must be something horribly wrong with the visitors because this thing chugs. Admittedly, I did write this with the 'I am learning ethos, let me try this' ethos so I was never too fussed about performance but this irks me now. After reading implementing Quant Lib, I wanted to implement a lower triangular matrix version of this model ( at the expense of path dependant results ) This ends up having O(n^2) time complexity and that mades a world of difference. You can actually get a ms result close to analytical black-scholes now!

Again on the performance theme, lets talk about the UI. I enjoyed learning aboout WPF and the MVVM. It seems like a half decent way to make a professional peice of software. However, I hate it. Maybe this is a result of what I was doing by visualising the tree, but boy did I struggle hack and GPT those edges [lines] in that connect the nodes. They are terrible and they contribute to the majority of the slowness of the UI itself. Also, events suck.

A note, I do actually know good git ettiquete. You just don't require it when you are working on a solo project and have stashes that you can use instead of branches.

## Furtherwork

- Asianing and Lookbacks seem to be exclusively an option for the BAPM.
- Adding dividends.
- Fixing that bloody, LayoutUpdated event that slows the UI down to a stand still!
- I would like to work more with rates and ivestigate a boostrapping tool and maybe something to construct a volatility surface (once I have finished reading Willmott 3.) This will likely be a new library and bastardise the UI further.

## Authors

* **Joe Osborne** [GitHub](https://github.com/JoesUsername98)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
Good luck making money off this demo project, have at it!

## Acknowledgments

* Thanks to Jack O'Mahoney for giving me my first shot as a quant with no experience.
* Thanks to Julien Hok, Xiaoqing Zhang for taking time to walk me through the maths and UI issues I was having, respectively.
* Thanks to all my other colleagues and friends in the Quant space for motivating me to love what I do and learn more everyday.
