# rendering-lab

A real-time rendering laboratory, demonstrating basic rendering techniques and configurable parameters.

> Project Status: This project is still WIP. Core architecture refactoring and feature iterations are ongoing.

---

## How to use

### Fly Camera Control

- `Left Click` in the scene to enter fly mode
- `W/A/S/D` : Move flying camera in the scene
- `Q/E` : Descend or elevate the camera
- `Esc`: Exit the fly mode and release the cursor

### Setting Panel

- Basic Settings

  - Background color: Configure the clear color of the viewport
- Camera Settings

  - FOV: Zoom of the camera
  - Sensitivity: Mouse movement sensitivity
  - Speed: Camera movement speed
- Light Settings:

  - Support:
    - 1 directional light
    - 4 point lights
    - 1 flashlight
  - One directional light, four point lights and one flashlight are avaliable
  - Enable/disable lights via checkboxes
- Shader Settings:

  - Phong Shading: The default per-pixel lighting model.
  - Toon Shading: Simple non-photorealistic rendering implementation.
    Parameters
    - Toon Threshold: Control the boundary between lit and shaded regions.
    - Toon Smoothness: Adjusts the gradient at the toon ramp transitions.

---

## TODO

- Optimize data structure and class design
- Improve model loading support (some formats currently unsupported)
- Enhance toon shading quality
- Better interaction

---

## Future Work

- Introduce advanced lighting models (e.g., PBR)
- Physics simulation
- Rigged Animation

---

## Third-party Resources

### Technical References

- [Learn OpenGL](https://learnopengl.com/)
- [Roystan&#39;s Toon Shader](https://roystan.net/articles/toon-shader/)

### 3D Models

- [Alicia Solid](https://3d.nicovideo.jp/alicia/)
- [Survival Guitar Backpack](https://sketchfab.com/3d-models/survival-guitar-backpack-799f8c4511f84fab8c3f12887f7e6b36)
- [Sphere](https://sketchfab.com/3d-models/sphere-b31b12ffa93a40f48c9d991b6f168f4d)

### Icons

- [Game Icons](https://github.com/game-icons/icons)
