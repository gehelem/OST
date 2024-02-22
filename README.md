[![CMake](https://github.com/gehelem/OST/actions/workflows/cmake.yml/badge.svg)](https://github.com/gehelem/OST/actions/workflows/cmake.yml)

See documentation site :

https://documentation.ostserver.fr/fr/presentation/index.html

# OST - Observatoire Sans Tête
/!\ be carefull, all this is a work in progess.
Very small parts are working, but you are welcome to come and play with me
(me = pretentious guy to think he can get away with this thing ...)

What am i trying to build here ?
--------------------------------
"Observatoire Sans Tête" is a joke with my friend René, a french word-to-word translation for "Headless Observatory".
OST is heavily inspired by two great projects  :
- https://astrophotoplus.gulinux.net/
- https://github.com/pludov/mobindi
 
I've been playing with both, i'm definitely convinced this is the way things should be done.
- Unfortunately 1 : both are talking Python/Javascript, wich is certainly fine and fancy, but completely unapproachable for my poor brain
- Unfortunately 2 : both are using hidden desktops to run PHD2- i don't want that.

My goal is multiple
- OST doesn't require any GUI, nor any desktop 
- OST must be declared as some kind of service
- OST is talking to hardware through Indi client protocol
- OST is talking to human through network, with any frontend understanding OST's language
- OST is modular
- OST must be autonomous

What do i expect from a module ?
--------------------------------
A module is loaded and controlled with OST.
A module doesn't have to care to render data.
A module achieves some specific tasks, like :
- Autofocus
- Guiding
- Sequencer
- Slewing/goto/ ...
- Meteo alerts
- Timelapses 
- Order beers
- (...)

Modules can communicate between each other (eg. autofocus <-> sequencer)

Why that way ?
--------------
I really want to make it easy for any buddy like me to build his own module, 
without having to care with GUi or rebuild what's have been build so many times.
Some way, i'd like OST to be the client equivalent of indiserver, where you can play to build your own driver and seemlessly make it work with any client.


What did i choose ?
-------------------
Choosing C++ was easy, has it's the only thing i understand a little.
Qt is helpling me a lot, with all it's programming fancies (not sure...).
Websocket is the first network protocol i've been able to use, it's doing what i expect, we'll keep that as long as it works...
Json descriptions for frontend/backend dialogs also seems to work like i expect ... (in fact, i expect to understand what i'm doing)
JQuery for frontend is also the first web frontend i've been able to play with. I also like the fact that it's quite old and lightweight, and provides a mobile version.

How i'm i trying to build my code ?
-----------------------------------
OST executable is a controller that instanciate modules.
Each module inherits the same communication structure with controller.
OST is also dealing with websockets messages, and translates incoming and outgoing messages from and to modules.

Where to play ?
---------------

Any Linux distribution should work, or any OS supporting Qt (windows ? iOs ??) 
I use Kubuntu at the moment, and also playing with various VM, RPi(4) and some small Nucs.
You don't need any desktop, all this should work without any graphic interface :
Command line is your friend, as long as you know how to build stuff that way.

Prerequisites to build
----------------------
This list below needs to be cleaned, some ppas might not be required anymore, i'll have a look at that someday...

```
sudo apt-add-repository ppa:mutlaqja/ppa
sudo apt-get update
sudo apt upgrade
sudo apt-get install build-essential cmake git libstellarsolver-dev libeigen3-dev libcfitsio-dev zlib1g-dev libindi-dev extra-cmake-modules libkf5plotting-dev libqt5svg5-dev libkf5xmlgui-dev libkf5kio-dev kinit-dev libkf5newstuff-dev kdoctools-dev libkf5notifications-dev qtdeclarative5-dev libkf5crash-dev gettext libnova-dev libgsl-dev libraw-dev libkf5notifyconfig-dev wcslib-dev libqt5websockets5-dev xplanet xplanet-images qt5keychain-dev libsecret-1-dev breeze-icon-theme indi-full libindi-dev cimg-dev libnova-dev build-essential nginx libgsl-dev wcslib-dev libcfitsio-dev qt5-default libcurl4-gnutls-dev libfftw3-dev imagemagick graphicsmagick libboost-all-dev
```

If you really want to play the "headless way" (wich is the goal), add this at the end of .bashrc to avoid Qt looking for graphical diplays 
`export QT_QPA_PLATFORM=offscreen`


News
====
09/12/2021
----------
Well, i think we'll keep on trying this way to deal with properties. I still don't understand Deufrai's implementation, but i've managed to use it ...
Big problem :
Someone on Webastro french forum told me that Indigo software is exactly doing what i'm trying to achieve ... OMG OMG OMG
https://www.indigo-astronomy.org/
What should i do ? I've tried : it's really a nice toy.
Problems with it :
- Not GPL
- Not Indi (despite partial compatibility)
- Only two (very active and obvioulsy talentuous) developers 
> I think i'll continue to play this game my way, just for fun.


28/11/2021
----------
We can instanciate modules dynamicaly, they can be built separately
Controller is responsible to store modules properties.
I've come to the opinion that this is not the good place to do so. We'll see.
We only have one specific module to control OST (just a skeleton at the moment).
My JQuery frontend (./HTML foler) is really messy.
It's really a pain for me to make this one do what i want. But it somehow works to test.
My great problem is modules properties, my implementation is uggly and not convenient.
I have to understand Deufrai's way to see things.
The Autofocus module is working quite well with indi simulators.
I have to say that Qt statemachines are really a great way to build things like that.


