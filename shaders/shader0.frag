#version 330 core

in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

uniform float blurSize;
const int blurIter=30;

void main()
{
	vec3 clrAvg=vec3(0, 0, 0);
	vec2 baseTexCord=TexCoord-vec2(blurSize/2.0, blurSize/2.0);
	
	for (float x=0; x<blurIter; x++)
	{
		for (float y=0; y<blurIter; y++)
		{
			clrAvg+=texture(ourTexture, vec2(baseTexCord.x+x*blurSize/blurIter, baseTexCord.y+y*blurSize/blurIter)).rgb;
		}
	}
	
	//color = texture(ourTexture, TexCoord);
	color=vec4(clrAvg/(blurIter*blurIter), 1.0f);
}

