# NEUROGENESIS 3D — native Android game

Standalone Android ARM64 game prototype. This project is deliberately separate from the Godot Phone editor in the repository root.

## Stack
- C++17 + Android NDK NativeActivity
- OpenGL ES 3 rendering
- Bullet Physics 3 rigid bodies and hinge constraints
- Dense neural controller (8 inputs → 12 hidden units → 4 motor outputs)
- Mutation-based evolutionary training loop

## Build
Open this `neurogenesis/` directory in Android Studio, or run:

```bash
gradle assembleDebug
```

The APK is produced at `app/build/outputs/apk/debug/app-debug.apk`. The GitHub Actions workflow builds the ARM64 debug APK and uploads it as an artifact.

## Controls
Tap the screen to pause/resume the controller. The current prototype runs a physical articulated creature in a 3D arena; the training episode advances automatically.

## Scope of this first build
This is the first native vertical slice: Android app lifecycle, GLES3 renderer, Bullet world, articulated rigid-body creature, neural motor controller, and evolutionary episode loop. It is not yet the full editor, map builder, creature designer, save system, or population-comparison laboratory described in the larger game brief.
