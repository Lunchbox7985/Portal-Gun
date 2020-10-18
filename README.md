# Portal-Gun
arduino setup for 3d printed portal gun

List of parts:

1. Arduino Nano (or clone) (https://www.amazon.com/ELEGOO-Arduino-ATmega328P-Without-Compatible/dp/B0713XK923/ref=sr_1_4?dchild=1&keywords=arduino+nano&qid=1602026870&sr=8-4)
2. Adafruit sound FX board (https://www.amazon.com/gp/product/B010M8UOR8/ref=ppx_yo_dt_b_asin_title_o07_s00?ie=UTF8&psc=1)
3. Neopixel Jewel x2 (https://www.amazon.com/gp/product/B0105VMT4S/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1)
4. Neopixel Ring 16 led version (https://www.amazon.com/dp/B08F9HSNSD/?coliid=IWRMIFVUICFMT&colid=3CGBY3S7A1KD5&psc=1&ref_=lv_ov_lig_dp_it)
5. Individual Neopixel (https://www.amazon.com/gp/product/B01D1FFVOA/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
6. 2 SPST Switches (https://www.digikey.com/en/products/detail/cw-industries/GRS-4011-0118/4425759)
7. 2 momentary buttons (i did one red and one black) (https://www.digikey.com/en/products/detail/e-switch/PS1023ARED/81776) (https://www.digikey.com/en/products/detail/e-switch/PS1023ABLK/82862)
8. Blue arcade button (https://www.digikey.com/en/products/detail/adafruit-industries-llc/3432/7349494)
9. Red arcade button (https://www.digikey.com/en/products/detail/adafruit-industries-llc/3430/7349492)
10. 2 inch speakers (https://www.amazon.com/gp/product/B01CHYIU26/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1)
11. 5 volt amplifier (https://www.amazon.com/gp/product/B07G4DLM9D/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
12. Micro usb board (https://www.amazon.com/gp/product/B07B5ZDLJY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
13. 330 ohm resistors x4
14. 3 Red 5mm LEDs, and another of any color (i used green) for the main power light.
15. Misc wires (i used scrap ethernet cable and some dupont jumpers i got on amazon)
16. USB battery bank (maybe not the smallest cheapest thing you can find, but it doesnt need to be huge)

1. i used those cheaper arduino clones i linked, should be interchangeable with any arduino nano.
2. i used the 2mb version, and with the sounds i provided it came in just under 2mb, there is a 16mb version if you want to use your own sound effects, or have them higher quality.
3. again, i got knock offs, but any neopixel compatible clone should do. I hvae had good luck so far with DIYMall brand from amazon.
4. same as above
5. i bought a pack of 100 as they are cheap, if you do other projects they can come in handy, but this only needs one.
6. the modified STL files i provided have cutouts for switchs of these dimensions specifically.
7. same as above
8. same as above
9. same as above
10. same as the switches, i designed the holes around these specific speakers.
11. a little less picky as the rest of the parts, this is designed around a usb battery bank, so any 5v amplifier should work. the one i used was about the cheapest available.
12. optional, as you coult hardwire power directly too the system. (more on that below)
13. might depend on the LEDs you get, but they should all be relatively the same. these just limit the current to keep the LEDs from burning out
14. these are for the base of the "fingers". in the game there were 3 red lights at the base as well and another 3 at the first joint. i opted for just the 3 for simplicity.
15. this is not exactly a power hog, so any wires should do fine.
16. with the exact parts listed above, minus the neopixel ring (i was originally going to just use 2 jewels), and only a single 8 ohm speaker hooked to the amp, with the amp at full volume i registered a little over an amp of current. i plan on running the 2 4-ohm speakers on each channel which will increase the current draw (i plan on testing it once i get it put together completly.) i have a battery which can supply 2 amps per usb, and a total of 3.2. if i notice power issues, i may hook the amp power to its own usb cord. i will try to remember to update this once i am done.


Power:

notice i ran power directly to the 5v pin of the arduino. i believe this bypasses the voltage regulator and powers it directly. you could power the arduino off the VIN pin with a higer voltage battery and run the lights, fx board, and amp off this pin, but it would be pushing the arduino to the limit. i chose to just power everytrhing seperatly from the usb battery bank so i dont fry the arduino. be careful though, as if you plug in your usb cable with this hooked up, it will attempt to output 5v on this pin and power everything else. the amplifier i linked can be turned off with the volume knob, so you should be safe that way

Button LEDs:

The LEDs in the blue and red arcade buttons could be powered off either A1 or A2. with the dupont connectors i used, it was more convienent to have 2 seperate pins, but you dont have to use both of them if you dont want to. I do however have the 3 red LEDs a little dimmer, so A0 is different than A1 and A2.

The Song:

The song button is programmed to play "Still Alive". i have not included this in the files for copyright purposes. you will have to find a copy yourself. I formatted it to OGG , mono and the lowest quality to get it down to 1.08MB there is a little room on the 2MB board for it to be a little bigger than that. As stated above, you can always get the 16MB FX board for just a couple bucks more. i believe there are 4 models, 2MB and 16MB, both with and without a 3.5mm jack. any of them should work, but i did not use the headphone jack.

2 Power switches:

the reason there are 2 power switches, is one is a true power switch that kills power to the whole circuit (so your battery doesn't drain when you aren't using it). The other switch is a simulated power switch. it powers the gun up and down (with power up and down sound effects). if you have the simulated power switch on when you turn the main power switch on, sometimes it just jumps straight into the idle hum, sometimes a couple of the lights turn green. either way the gun works as expected once you push a button.

If you have any questions, comment on thingiverse, i might not see it imeediatly, but i will try my best to respond.
