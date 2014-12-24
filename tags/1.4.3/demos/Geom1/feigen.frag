// Trivial semi-random stripe shader.

void main()
{
  vec3 col = vec3(1.00 + 4.00*gl_TexCoord[0].x + 2.00*gl_TexCoord[0].y,
		  2.00 + 1.00*gl_TexCoord[0].x + 4.00*gl_TexCoord[0].y,
		  4.00 + 2.00*gl_TexCoord[0].x + 1.00*gl_TexCoord[0].y);

  col = fract(col);
  col = floor(10.0*col)*0.1;

  for (int i = 0; i < 20; ++i) {
    col = 4.0 * col * (1.0 - col); // use 4.0 for pure chaoss
  }

  // gl_FragColor = vec4(col, 1.0);
  gl_FragColor = mix(vec4(col, 1.0), gl_Color, .5);
}
