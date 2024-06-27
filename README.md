# Paper presentation: "iCheat: A Representation for Artistic Control of Indirect Cinematic Lighting"

Group 20: Cristina Stoleriu, Gideon Bot, Zixuan Han

This demo showcases the memory optimization methods described in the paper. Additionally, we implemented a very simple indirect illumination model based on ray tracing.

The most important files are:
- `main.cpp` - contains the main game loop logic, manages the UI and scene. Users can change the scale and offset of indirect illumination for pairs of meshes and observe the changes to the T matrix. The result of the applied edits can be viewed by rendering the scene to a file using the **Render to file** button.
- `illumination.cpp` - Computes direct and indirect illumination when the **Render to file** button is pressed. It also contains the Haar wavelet transform logic.
- `sparseMatrix.cpp` - A simple sparse matrix implementation used for the transport matrix T.