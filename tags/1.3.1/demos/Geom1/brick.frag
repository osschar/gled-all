//
// Fragment shader for procedural bricks
//
// Authors: Dave Baldwin, Steve Koren, Randi Rost
//          based on a shader by Darwyn Peachey
//
// Copyright (c) 2002-2006 3Dlabs Inc. Ltd. 
//
// See 3Dlabs-License.txt for license information
//

uniform vec3  BrickColor;  // (1.0, 0.3, 0.2)
uniform vec3  MortarColor; // (0.85, 0.86, 0.84)
uniform vec3  BrickSize;   // (0.3, 0.3. 0.15)
uniform vec3  BrickPct;    // (0.9, 0.9, 0.85)

varying vec3  MCposition;
varying float LightIntensity;

void main()
{
    vec3  color;
    vec3  position, useBrick;
    
    position = MCposition / BrickSize;

    if (fract(position.z * 0.5) > 0.5)
        position.xy += 0.5;

    float fpy = fract(position.y * 0.25);
    if      (fpy > 0.75) position.x += 0.75;
    else if (fpy > 0.50) position.x += 0.50;
    else if (fpy > 0.25) position.x += 0.25;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y * useBrick.z);
    color *= LightIntensity;
    gl_FragColor = vec4(color, 1.0);
}
