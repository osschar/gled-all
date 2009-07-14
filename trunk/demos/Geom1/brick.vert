//
// Vertex shader for procedural bricks
//
// Authors: Dave Baldwin, Steve Koren, Randi Rost
//          based on a shader by Darwyn Peachey
//
// Copyright (c) 2002-2006 3Dlabs Inc. Ltd. 
//
// See 3Dlabs-License.txt for license information
//

uniform vec3 LightPosition; // (0, 0, 10)

const float SpecularContribution = 0.3;
const float DiffuseContribution  = 1.0 - SpecularContribution;

varying float LightIntensity;
varying vec3  MCposition;

void main()
{
    vec3  ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
    vec3  tnorm      = normalize(gl_NormalMatrix * gl_Normal);
    vec3  lightVec   = normalize(LightPosition - ecPosition);
    vec3  reflectVec = reflect(-lightVec, tnorm);
    vec3  viewVec    = normalize(-ecPosition);
    float diffuse    = max(dot(lightVec, tnorm), 0.1);
    float spec       = 0.0;

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        spec = pow(spec, 2.0);
    }

    LightIntensity  = DiffuseContribution * diffuse +
                      SpecularContribution * spec;

    MCposition      = gl_Vertex.xyz;
    gl_Position     = ftransform();
}
