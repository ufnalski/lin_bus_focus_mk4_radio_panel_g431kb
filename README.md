# Ford Focus Mk4 (2020) radio panel [STM32G431KB]
The part number is [JX7T-18K811-CC](https://allegro.pl/listing?string=JX7T-18K811-CC).

![Ford Focus Mk4 radio panel in action](/Assets/Images/focus_mk4_radio_panel_in_action.jpg)
![Ford Focus Mk4 radio panel PCB](/Assets/Images/focus_mk4_radio_panel_pcb.jpg)
![Ford Focus Mk4 radio panel frames](/Assets/Images/focus_mk4_radio_panel_lin_bus_scan.JPG)
![Ford Focus Mk4 radio panel serial monitor](/Assets/Images/focus_mk4_radio_panel_tabby.JPG)

> [!NOTE]
> The radio panel is able to synchronize at 9600 baud as well as 10400 baud. I don't have the master device to verify its native baud rate. The Google AI Overview suggests that 9600 is used in some Ford cars, whereas 10400 is present in some Opel models. 

> [!IMPORTANT]
> The radio panel implements the enhanced checksum - the panel is a LIN 2.x client. You can easily identify whether your device uses LIN 1.x or 2.x by triggering its response and comparing the last byte to the one calculated with the help of an online calculator, such as the one available [here](https://linchecksumcalculator.machsystems.cz/). Compare it with the panel from [Focus Mk3](https://github.com/ufnalski/lin_bus_focus_radio_panel_g431kb).

# Pinout
![Ford Focus Mk4 radio panel pinout](/Assets/Images/focus_mk4_radio_panel_pinout.jpg)

# Missing files?
Don't worry :slightly_smiling_face: Just log in to MyST and hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# What's inside?
* [MLX80051C LIN system basis chip](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/511/MLX80050_51_30_31_Rev13_Mar2020.pdf) (Melexis)
* [R5F109BCJ 16-bit microcontroller](https://www.digikey.pl/en/products/base-product/renesas-electronics-corporation/20/R5F109/295369) (Renesas)

# Scanners
* [LIN bus RX scanner](https://github.com/ufnalski/lin_bus_rx_scanner_g431kb)
* [LIN bus TX scanner](https://github.com/ufnalski/lin_bus_tx_scanner_g431kb)

# Call for action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2025_dzien_popularyzacji_matematyki/Dzien_Popularyzacji_Matematyki_2025.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [bitluni's lab](https://www.youtube.com/@bitluni), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), [Max Imagination](https://www.youtube.com/@MaxImagination), [Nikodem Bartnik](https://www.youtube.com/@nikodembartnik), [Stuff Made Here](https://www.youtube.com/@StuffMadeHere), [Mario's Ideas](https://www.youtube.com/@marios_ideas), [Aaed Musa](https://www.aaedmusa.com/), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Human-machine interfaces - do try them at home :exclamation:

200+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned :sunglasses:
