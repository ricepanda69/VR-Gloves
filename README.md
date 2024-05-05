VR Gloves
======

![](/Media/vrglove.gif)

## About
This project focuses on developing a simple virtual reality (VR) glove designed to bring basic hand gestures into virtual environments with a focus on wireless connectivity and interactability, allowing users to interact with VR content in a more intuitive way. 

## Objective
The primary objective of this project is to create an easy-to-use VR glove that translates basic hand and finger movements into VR interactions. 

## Current Progress / Key Features
* **Wireless Connectivity:** The glove connects to VR systems wirelessly via WiFi, offering freedom of movement without the constraints of cables.
* **Hand Gestures** The glove can mimic finger movements, which can be used to interact with VR objects and environments.
* **Hand Rotation Detection:** Incorporates sensors to track basic hand rotation, enabling users to perform simple rotational gestures in VR.
* **Aesthetic Design:** The glove has a clean, streamlined design with no visible wiring, ensuring it is visually appealing and comfortable to wear.

## Future Development
The long-term goal is to create a more advanced VR glove with enhanced functionality, including better finger tracking and more accurate position tracking.

## Most Interesting Stuff
### Images of Final Prototype
![](/Media/IMG_1562.jpg)
![](/Media/IMG_1563.jpg)
![](/Media/IMG_1564.jpg)
![](/Media/IMG_1566.jpg)
![](/Media/IMG_1567.jpg)

### Video of Final Prototype
https://youtu.be/PU5o2eUM6zc

---

Process
======
## Research and Conceptualization
I've done a lot of work with VR systems, but I've never tried making my own VR controller before. I've always disliked most VR controllers; they never felt *right*. Even with the Vale Index's finger tracking, I still have to hold a controller that feels too clunky and unnatural, and most VR controllers don't even come with finger tracking either. I've heard of VR gloves being developed before, so I figured I'd take my best shot at it.

[This](https://github.com/LucidVR/lucidgloves) was my original inspiration for wanting to make VR gloves. This repo is all set up with support for SteamVR (the go-to VR software), and has instructions on 3D printing parts and setting up the electronics and firmware, etc. What I didn't like about it was the fact that it was rather big, and I wanted something as low-profile as possible. What made the design so big were the 3D printed parts and the potentiometers used to detect finger movements. I decided that, if I want to be as low-profile as possible, I'd need a different approach, so I decided I wanted to use flex sensors. Also, I was going to need to gather code and/or write my own in order to suit my design, and to be able to interface with Unity instead of SteamVR. 

Another key feature I wanted for this project was for it to be wireless. I deciced to use Bluetooth since connecting peripherals to a PC (for PC VR applications) via Bluetooth is commonplace. I went with BLE (Bluetooth Low Energy) since I lready had a module that supported it.

Lastly, I wanted it to be able to tack position without the need for external reference points, such as base stations. ([Here's](https://www.uploadvr.com/how-vr-tracking-works/) an article that explains how it works; scroll down to the section titled "SteamVR “Lighthouse” (HTC Vive)"). In order to do this, I thought that using an IMU (Inertial Measurement Unit) would help, and it does, but there's an inherent issue called "drift". An IMU usually has a gyroscope and an accelerometer (6DoF), but sometimes it also has a magnetometer (9Dof). If an IMU is used for position tracking, the biggest source of drift is the accelerometer. In order to get position from acceleration, two integrations are needed (mathematical integration, y'know, the opposite of derivation). Each integration introduces some error, which only increases after each as well. Combined with noise, even with calibration, correct positional tracking is lost after a short amount of time. Another source of drift is the gyroscope; most hobby-grade gyroscopes can accurately measure absolute orientation, but drift can be caused by DC bias and angular random walk (ARW; basically noise). However, this usually is fixed via calibration. Nonetheless, after some research, I found that sensor fusion can help a lot with drift from both sources, so I figured it would be fine to try it out. Many sources on the internet ([here](https://forum.arduino.cc/t/position-tracking-with-imu-sensor-and-arduino/700371/6), [here](https://forum.arduino.cc/t/hand-positioning-with-imu/1159698/5), [here](https://www.reddit.com/r/arduino/comments/131ouxf/comment/ji1iou0/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button), among many places) have pointed out that it's just not really possible to perfrom position tracking with only an IMU, but I'm too stubborn to listen to random people from the internet and wanted to try it anyway. 

## First Prototype
Initially, I didn't want to focus on the design or aesthetic; I wanted to focus on the "backend" stuff, like connecting via Bluetooth and reading IMU data and processing it with sensor fusion algorithms. However, the very first thing I had to decide was what microcontroller to use. This was actually very easy for me; my go-to answer is Teensy. Specifically, I had a [Teensy 4.0](https://www.pjrc.com/store/teensy40.html), which normally runs at 600MHz, but can run slower, and can even be overclocked to over 1GHz. As I've read before, the faster the better for sensor fusion algorithms, plus it's a pretty small board, so I thought it would be perfect. Choosing the other components were pretty easy too; just choose the "best" ones for each purpose (or the next best one based on availability), so I went with the [Adafruit TDK InvenSense ICM-20948 9-DoF IMU](https://www.adafruit.com/product/4554) for my IMU, and I only had this option for [flex sensors](https://www.adafruit.com/product/1070). Lastly, for BLE, I went with an [HM-10 Bluetooth 4.0 V3 Module](https://www.microcenter.com/product/656635/inland-ks0455-hm-10-bluetooth-40-v3-module) - this one I picked up locally. Since it's going to be wireless, it's going to need a battery pack of some sort; I laready have a bunch, and one of them actually has a USB port, so I can easily connect it to my Teensy via USB cable.

[Here](/Media/prototype1.png) is the Fritzing diagram of the first protoype that I made.

Here is a list of links to a bunch of images of the first prototype:
* [Image 1](/Media/IMG_1518.jpg)
* [Image 2](/Media/IMG_1519.jpg)
* [Image 3](/Media/IMG_1520.jpg)
* [Image 4](/Media/IMG_1521.jpg)
* [Image 5](/Media/IMG_1522.jpg)
* [Image 6](/Media/IMG_1523.jpg)
* [Image 7](/Media/IMG_1524.jpg)

### Initial Testing
Reading data from the IMU was fairly simple, granted there was a tutorial on how to do sensor fusion from [Adafruit](https://learn.adafruit.com/how-to-fuse-motion-sensor-data-into-ahrs-orientation-euler-quaternions/overview). The tutorial includes everything needed, including a link to software for calibrating the magnetometer (if the IMU has one). The only thing I had to do was send the data over to Unity over Bluetooth. In order to do that, I just needed to set up serial communication with my HC-10 module like so:
```
Serial1.begin(115200)
```
where Serial1 is the another hardware serial port that's available on Arduinos / Teensy's (pin assignments vary per board). Then I send data (as euler rotation values) like this:
```
char msg[20] = "0123456789ABCDEF\r\n";
sprintf(msg, "%.02f,%.02f,%.02f,", roll, pitch, heading);
Serial1.print(msg);
Serial1.flush();
```
And then in Unity it'd be read like this:
```
float[] values = Array.ConvertAll(msg.Split(','), float.Parse);
obj.transform.rotation = Quaternion.Euler(new Vector3(values[0], values[1], values[2]));
```
where `obj` is the GameObject (a cube for this stage of development). This code just applies the rotation values passed via Bluetooth to the object.

It took a while to get the BLE module to work, but after a while it finally was doing what I wanted it to do. It was a mix of trial-and-error and researching about the module itself in order to understand exactly what it needed in order to work. Originally, I wanted to try using Bluetooth serial to transmit data, but that apparently isn't supported with the HC-10. I used [this repo](https://github.com/Joelx/BleWinrtDll-Unity-Demo/tree/main/Assets/Scripts) in order to get a basis for connecting BLE devices to Unity and edited it as needed for my use case. I also used a Bluetooth scanner app on my phone to try and read values there as an additional debugging step, and I found that if I tried to sending more than 20 bytes, the message would get cut off, hence `char msg[20] = "0123456789ABCDEF\r\n";`. This was an issue since I noticed that, with the 20 byte limitation, I could only send floats that have a precision of 2 decimal places; not only that, but I planned on attaching more sensors, so 20 bytes just wasn't going to cut it, meaning I had to try some other way. 

Here is the prototype in action. I used Unity to write up a quick program to test out the prototype.
https://youtu.be/N9mRMXFwxLk

Arduino sketches can be found [here](/Arduino/prototype/prototype.ino)

## Second Prototype
My second prototype involved adding flex sensors, as well as changing up the hardware a bit. I opted for a [Adafruit QT Py ESP32 Pico](https://www.adafruit.com/product/5395) since it has an ESP32 chip onboard, which is capable of WiFi (with which I decided to replace BLE/Bluetooth since I can send much more data more quickly). It also has a Qwiic port, meaning my IMU (which also happens to have a Qwiic port) is more or less plug-n-play, which means one less thing I have to think about. I also got a [16 Channel Analog Multiplexer](https://a.co/d/ePp38Vf) since the QT Py board I got doesn't have enough GPIO pins for me to use for the flex sensors. 

Also, I moved on from breadboard-on-battery-pack to an actual wearable glove. I just got some black gloves from a local store to serve as the base for all of my electronics, and I ordered one off of Amazon to put over the base glove to hide all the electronics (specifically, [this one](https://a.co/d/1AAIXlO) because I thought it looked cool).

I stitched some fabric together to make little sleeves for the flex sensors to slide into, and then attached them to the base glove with some flexible iron-on adhesive. I based the placement of each flex sensor off of [this configuration](https://www.researchgate.net/figure/Component-placement-a-Placement-of-the-ten-flex-sensors-Sensor-denominations-X-Y_fig1_310499445) I found through some Google searching. I also made little pockets for the QT Py, IMU, and multiplexer. All wires were soldered to each component directly; I didn't use a protoboard or anything to minimize the amount of literal "hard"-ware that could prevent glove flexibility. 

Images of what I produced can be found in the list of links below:
* [Image 1](/Media/IMG_1554.jpg)
* [Image 2](/Media/IMG_1555.jpg)

This prototype didn't work at all; I *was* able to make the QT Py communicate with Unity by following the tutorials [here](https://learn.adafruit.com/adafruit-qt-py-esp32-pico) and using the [Uduino](https://marcteyssier.com/uduino/) plug-in, but I wasn't reading any sort of analog input from the flex sensors. It turns out that I actually wired them wrong; I had forgotten to set them up with a voltage divider and instead, I just connected one pin to GND and the other directly to the multiplexer inputs. Additionally, I found that the sleeves I made were actually fraying and falling apart, so I just decided to start over again. 

## Third Prototype
I figured that the easiest thing to start with improvements were the sleeves; this time, I hemmed the edges and added fabric glue to the any edge I trimmed to prevent fraying. I still used iron-on adhesive to attach the sleeves to the glove, however. For the glove, I noticed that it didn't fit my fingers very well (I have pretty long fingers), so I cut holes at the tips of each finger, cut the fingers off of the glove I wasn't using, and then used iron-on adhesive to attach the cut fingers to the fingertip-less glove to extend the base glove's finger length. The glvoe fit pretty nicely, and it held together pretty well. 

I rewired the flex sensors so that 3.3V went into one pin and GND and a signal wire going to the multiplexer out the other pin. GND was connected to a 33k Ohm resistor (flex sensors are rated to be from ~25k Ohms to ~100k Ohms, so I chose something that I had handy that was in between those values). Additionally, I wired 5V from the QT Py to the multiplexer. I based my wiring on the following forum posts: [here](https://forum.arduino.cc/t/multiplexing-thermistors-with-only-one-reference-resistor/614969) and [here](https://forum.arduino.cc/t/problem-using-multiplexer-and-fsr/1125028). 

[Here](/Media/prototype2.png) is a Fritzing diagram of what it looks like. There's only 5 flex sensors in this diagram for ease in viewing, but I wired 11 flex sensors total.

Here is a list of images to show what the third prototype looks like:
* [Image 1](/Media/IMG_1562.jpg)
* [Image 2](/Media/IMG_1563.jpg)
* [Image 3](/Media/IMG_1564.jpg)
* [Image 4](/Media/IMG_1566.jpg)
* [Image 5](/Media/IMG_1567.jpg)

### Final Testing
Using Uduino and a [Sci-Fi VR Hand](https://assetstore.unity.com/packages/3d/characters/sci-fi-vr-hand-134394) asset from the Unity Asset Store, I was able to get some basic hand gestures working. I set up a [calibration script](/Assets/Scripts/GloveController.cs) in Unity, which I explain in a little detail below:

All of the following code is run once per loop in Update(); similar to loop() for Arduino, Update() in Unity runs once per frame.

Here, I assign values (which was previously defined), and define raw values for each finger. Also, a float `dt` is defined as the difference current time `Time.time` and initial time `t` (also previously defined). I use this variable later to determine different stages of calibration.
```
values = Array.ConvertAll(msg.Split(','), float.Parse);

float index = values[3] + values[4];
float middle = values[5] + values[6];
float ring = values[7] + values[8];
float pinky = values[9] + values[10];
float thumb = values[11] + values[12];

float dt = Time.time - t;
```

First, I remind the user that calibration is starting, and to first make a fist so that the program can record the minimum values. Then, for 10 seconds, the program records the lowest value for each finger. 
```
if (dt < 5f)
{
    text.text = "Beginning Calibration. Please make a fist and hold that pose.";
}
else if (dt >= 5f && dt < 15f)
{
    text.text = "Now Calibrating...";

    if (index < index_min) index_min = index;
    if (middle < middle_min) middle_min = middle;
    if (ring < ring_min) ring_min = ring;
    if (pinky < pinky_min) pinky_min = pinky;
    if (thumb < thumb_min) thumb_min = thumb;
}
```

Next, I do the same thing, but with a relaxed, open hand. I also add up values for hand angles which will be averaged out at the end and used as an offset.
```
else if (dt >= 15f && dt < 20f)
{
    text.text = "Now, please relax your hand and place it on a flat surface.";
}
else if (dt >= 20f && dt < 30f)
{
    text.text = "Resuming Calibration...";
    if (index > index_max) index_max = index;
    if (middle > middle_max) middle_max = middle;
    if (ring > ring_max) ring_max = ring;
    if (pinky > pinky_max) pinky_max = pinky;
    if (thumb > thumb_max) thumb_max = thumb;

    angleX_avg += values[1];
    angleY_avg += values[2];
    angleZ_avg += values[0];
    n++;
}
```

Finally, calibration completes, and all angle offsets get averaged.
```
else if (dt >= 30f)
{
    text.text = "Calibration complete.";

    angleX_avg /= n;
    angleY_avg /= n;
    angleZ_avg /= n;

    isCalibrate = false;
}
```

Here, normally it's `obj.transform.rotation = Quaternion.Euler(new Vector3(values[0], values[1], values[2]));` but there were some initial values for the hand to be oriented correctly for the demo, and some angle values didn't match their corresponding axes, so through trial-and-error I was able to rearrange the values to have correct axes.
```
obj.transform.rotation = Quaternion.Euler(new Vector3(-values[1] - angleX_avg + angleX0, values[2] - angleX0 + angleY0, values[0] - angleZ_avg + angleZ0));
```

Lastly, I take the inverse interpolation between the recorded min and max values for each finger with the current finger value and apply that to the VR hand controller.
```
controller[1] = Mathf.InverseLerp(index_max, index_min, index);
controller[2] = Mathf.InverseLerp(middle_max, middle_min, middle);
controller[3] = Mathf.InverseLerp(ring_max, ring_min, ring);
controller[4] = Mathf.InverseLerp(pinky_max, pinky_min, pinky);
controller[0] = Mathf.InverseLerp(thumb_max, thumb_min, thumb);
```

Here's a video showing the glove in action:
https://youtu.be/9Z-j8VQz3_A
