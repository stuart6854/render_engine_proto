# Render Engine Prototype

Just a repository for me to play with designing a Render Engine. Maybe something will come of it (*or maybe not*).

## Terminology

- Engine = The library for provided by this repository
- Host = The application/library consuming the Engine
- Graphics Resource - eg. Mesh, Texture, Material

## Thoughts

- Let the Engine decide when to load/unload graphics resources.
  - Host, when registering graphics resources, would provide requires information about resource.

- Mesh LODs
  - https://computergraphics.stackexchange.com/questions/1438/what-is-the-state-of-art-in-geometric-lod-in-games