# UnrealSplat - 3D Gaussian Splatting for Unreal Engine

![Unreal Engine Version](https://img.shields.io/badge/Unreal%20Engine-5.5-purple.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%2064--bit-blue.svg)
![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)
![PRs Welcome](https://img.shields.io/badge/PRs-welcome-orange.svg)

UnrealSplat is a plugin for Unreal Engine 5.5 that enables high-performance, real-time rendering of 3D Gaussian Splatting models. It leverages Unreal's built-in Niagara system to render scenes with up to 2 million splats efficiently.

<img width="1101" height="977" alt="image" src="https://github.com/user-attachments/assets/3c1dbbb9-1692-40de-8886-48eea588dbd3" />
Example Screenshot from within the Unreal Engine's Editor using this Plugin. The 3DGS Model is from https://jonbarron.info/mipnerf360/.

---

## ✅ What can the Plugin Do?
The Plugin is still in relatively early development.

* **Niagara-Powered Rendering**: Utilizes Unreal Engine's Niagara system for rendering, ensuring high performance and integration with the engine's VFX pipeline.
* **Large Model Support**: Efficiently renders models with up to 2 million splats.
* **Texture-Based Splat Storage**: Splat properties are stored in textures and loaded on-demand by Niagara for optimal memory and performance.
* **High Performance**: Designed for real-time applications, including VR and interactive walkthroughs.
* **Simple UI**: A straightforward user interface for quick and easy model loading.

---

## 🚧 Current Limitations & Known Issues

This plugin is currently in active development. Please be aware of the following limitations:

* **Transformations**: Moving, rotating, or scaling the splat actor in the world is not fully supported. While the actor can be moved, the rendering may break.
* **Spherical Harmonics (SH)**: Support for spherical harmonics is a work-in-progress (WIP) and is currently disabled.
* **Model Size**: Models significantly larger than 2 million splats may not render correctly.
* **File Format**: Only `.ply` files from standard Gaussian Splatting training outputs are currently supported.

---

## 🔧 Installation

1.  **Clone the plugin into the Plugins Folder of your Unreal Project**. Itincludes pre-compiled binaries for **Windows 64-bit**.
2.  **Restart your project.** The plugin should be enabled automatically (You might need to rebuild the plugin from your Projects Visual Studio Environment). You can verify this under **Edit > Plugins**.

---

## 🚀 How to Use

1.  **Place Your Model**: Copy your `.ply` model file somewhere inside your project's `Content/Models` folder (e.g., `Content/Models/my_model.ply`).
2.  **Open the UnrealSplat UI**: A new button named **"UnrealSplat"** will appear on the main toolbar in the editor. Click it to open the loading UI.
3.  **Enter the File Path**: The UI will prompt you for a string. Enter the path to your model **relative to the project's `Content/Models` folder**.
    * For example, if your model is at `[YourProject]/Content/Models/my_model.ply`, you would enter:
        ```
        my_model.ply
        ```
4.  **Load Model**: Press the Render button in the UI. The plugin will create the necessary assets and spawn an actor in your scene to render the model.



---

## 🤝 How to Contribute

This is an open-source project and contributions are welcome! If you want to help fix a known issue or add a new feature, please follow the standard GitHub Fork & Pull Request workflow. For bugs or suggestions, please open an issue.

---

## 📜 License

This project is distributed under the MIT License. See the `LICENSE` file for more information.

This Project was developed as part of my working student job at the Chair of Computer Graphics and Visualization at the Technical University of Munich.

---

## 📬 Contact

Jonas Itt: [https://github.com/JI20](https://github.com/JI20)
Chair of Computer Graphics and Visualization, Technical University of Munich: [https://www.cs.cit.tum.de/cg/cover-page/](https://www.cs.cit.tum.de/cg/cover-page/)

## 🙏 Acknowledgments

This project was made possible thanks to the pioneering work of others in the community. The development of this plugin was heavily inspired by, and adapts code from, the following incredible projects.

* **Original 3D Gaussian Splatting Paper**
    * Kerbel, B., Kopanas, G., Martin-Brualla, R., & Drettakis, G. (2023). *3D Gaussian Splatting for Real-Time Radiance Field Rendering*.
    * [Project Page](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/)

* **XScene-UEPlugin by xverse-engine** (Apache 2.0 License)
    * This project provided a foundational understanding of integrating advanced rendering techniques into Unreal Engine. Some code and general UX patterns were adapted from here.
    * [GitHub Repository](https://github.com/xverse-engine/XScene-UEPlugin)

* **GaussianSplattingForUnrealEngine by Italink** (MIT License)
    * This repository served as a key reference for the implementation of the core Gaussian Splatting rendering logic.
    * [GitHub Repository](https://github.com/Italink/GaussianSplattingForUnrealEngine)
