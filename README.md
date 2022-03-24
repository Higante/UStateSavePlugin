# USaveStatePlugin

Unreal Engine 4 plug-in that adds a way to save and load a state of a level within an exterior
file, using internal serialize methods to do so. It has been programmed with ROS Integration in
mind.

## Supported Unreal Engine Versions
Unreal Engine **4.22 and upwards**.

## Dependencies
This plug-in requires two other plug-ins to function correctly:

1. [UROSBridge](https://github.com/robcog-iai/UROSBridge)
2. [UROSWorldControl](https://github.com/robcog-iai/UROSWorldControl)

## Usage
You can put this plug-in into your Plugin Folder of your project, which will give you access to 
an **ASaveStateActor**. Add this one to the Level, it will add related ROS-Services to the mix
which can be called using the rosservice call function for example.

## Optimization Notes WIP
If you desire to reduce the filesize which might result from the saving method here, which can
balloon depending on how many items one has in the room, consider using the `SaveGame` MetaData
Flag for your UProperties. 

The SaveState Editor will then serialize those properties which do have said Flag.
A nice function to have if you would like to optimize it. Even more so if you wish to
take the Type sizes into consideration and only save what's necessary.

This Plugin uses a native Archive Proxy which serializes UObjects into convertible TArray<uint8>
filled with basically String.

## TO-DO

## Other Documentation

1. [Serialization References](Documentation/Serialization.md)

## Videos

- Video01 [Mikrowelle Showcase] Main
- Video02 [Zuruecksetzung Sliced Food Video] Secondary
- Video03 [ROBOTO] Tertiary
