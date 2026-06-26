# XrInteract

A plug-and-play **Unreal Engine plugin** that uses **OpenXR** that provides **hand tracking, gesture recognition, controller tracking, and runtime Mixed Reality (AR/VR) switching**.

Built for Unreal Engine developers who want to quickly integrate modern XR interactions without writing platform-specific code. XrInteract leverages the OpenXR standard, making it compatible with a wide range of XR headsets that support the required extensions.

---

## ✨ Features

* ✋ Real-time hand tracking using OpenXR
* 🤌 Built-in gesture recognition
* 🎮 Controller tracking support
* 🥽 Runtime AR/VR (Mixed Reality) switching
* 🔄 Automatic support for hand and controller interactions
* ⚡ Lightweight and easy to integrate
* 🌍 Cross-device compatibility through OpenXR

---

## Supported Devices

Tested on:

* ✅ Varjo XR-4
* ✅ Meta Quest 3

Compatible with any headset supporting:

* OpenXR Runtime
* OpenXR Hand Tracking

---

## Requirements

* Unreal Engine 5.6+
* OpenXR plugin enabled
* OpenXR Runtime installed
* Hand Tracking enabled on your target device

> **UE 5.7 Note:**
> XrInteract has been tested on Unreal Engine 5.7. Due to an API change, you may need to pass the additional `bool IsTracked` parameter to the updated function if you encounter a compilation error.

---

## Usage

### Hand Tracking

Add the provided hand tracking component to your Pawn or Actor to receive real-time hand joint data.

### Gesture Recognition

Use the included gesture recognition component to detect supported hand poses and gestures.

### Controller Tracking

Enable controller tracking to support traditional VR controllers alongside hand tracking.

### Runtime MR Switching

Switch between:

* Virtual Reality (VR)
* Mixed Reality / Augmented Reality (MR/AR)

during runtime without restarting your application.

---


## Unreal Engine Support

* ✅ UE 5.6
* ✅ UE 5.7



