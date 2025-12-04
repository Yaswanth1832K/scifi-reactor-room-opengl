# Sci-Fi Reactor Room – OpenGL 3D Scene

An animated **3D sci-fi reactor room** built using **C++ and OpenGL/GLUT**.

The scene features:

- A glowing **reactor core** with pulsating emissive lighting  
- Multiple **rotating energy rings** around the core  
- **Workers (humans)** walking around the reactor and machines  
- **Control machines** with holographic screens  
- **Ceiling LEDs**, **floor lights**, and dark **sci-fi walls**  
- Particle-like energy orbs orbiting the reactor  
- A custom **matrix stack** for translations, rotations, and scaling  

This project was created as part of a **Computer Graphics** course.


## Features

- Custom implementation of:
  - `custom_push_matrix` / `custom_pop_matrix`
  - `custom_translate`, `custom_rotate`, `custom_scale`
- Animated:
  - Reactor core glow
  - Rotating torus rings
  - Floating holograms
  - Particle system around the core
  - Moving workers and machines
- Multiple scene components:
  - Floor, walls, ceiling
  - Ceiling LEDs grid
  - Floor lights around the reactor
  - Control consoles and operators

## Controls

- `ESC` – Exit the program  
- `Space` – Pause / resume animation  
- `v` – Toggle wireframe / solid rendering  
- `+` – Increase animation speed  
- `-` – Decrease animation speed  
- `l` – Toggle floor lights on/off  

## Requirements

- **C++ compiler**
- **OpenGL** libraries
- **GLUT** (e.g., `freeglut`)

On Windows:
- You can use **Visual Studio**, **Code::Blocks**, or **MinGW**  
- Make sure `opengl32.lib`, `glu32.lib`, and `glut/freeglut` libs are linked

On Linux (example):
```bash
sudo apt-get install build-essential freeglut3-dev
