/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit ---

Copyright (C) 2009 Xavier Vergu�n Gonz�lez <xavierverguin@hotmail.com>
                                           <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
GLSL shaders added by Jose Luis Cercos Pita.
--------------------------------------------------------------------------------
*/

// --------------------- SkyX moon material ------------------------
// OUT
varying vec4 UVYLength;
// UNIFORM
uniform mat4 uWorldViewProj;
uniform mat4 uWorld;
uniform vec3 uSkydomeCenter;

void main()
{
    // Clip space position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// World position
	vec3 ObjectSpacePosition = (uWorld * gl_Vertex).xyz - uSkydomeCenter;

    // UV
    UVYLength.xy = gl_MultiTexCoord0.xy;
    // Y
    UVYLength.z  = ObjectSpacePosition.y;
    // Length
    UVYLength.w  = length(ObjectSpacePosition);
}

