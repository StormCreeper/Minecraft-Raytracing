# Minecraft-Raytracing
This is my attempt at making a minecraft clone using exclusively voxel raytracing and procedural textures.  
I'm using one primary ray for the base block color, and two reflection and refraction rays for the visual effects.  
The map generation is done procedurally in a compute shader, to allow very fast generation of vast worlds.  
The textures are also procedural, computed in the fragment shader, so each block can have a different appearance, and be at a virtually infinite resolution.  

The game currently doesn't support any acceleration structure, and the whole world is stored in one big OpenGL 2D texture, so it isn't infinite. I also know the procedural textures takes quite the GPU load, but it was just a fun experiment to see how far I could go with fully procedural worlds running on the GPU.

## Images
![Procedural terrain generation 1](images/gen_1.png)  
*Procedural terrain generation on the GPU*  
<br />
<br />
<br />
![Procedural terrain generation 2](images/gen_2.png)  
*Showcasing rivers*  
<br />
<br />
<br />
![Animated water](images/water.png)  
*Animated water with reflections*  
<br />
<br />
<br />
![Minivoxels 1](images/minivox_1.png)    
*I also implemented sub-voxels of arbitrary size, that we can load from magicavoxel*  
<br />
<br />
<br />
![Minivoxels 2](images/minivox_2.png)    
*Another detailled log*  
<br />
<br />
<br />
![Procedural sky](images/sky.png)    
*Procedural sky using noise to fake clouds*  
<br />
<br />
<br />
![Procedural bricks](images/procedural_1.png)    
*Showcasing procedural bricks*  
<br />
<br />
<br />
![Procedural ore veins](images/procedural_2.png)    
*and ores*  
<br />
<br />
<br />

