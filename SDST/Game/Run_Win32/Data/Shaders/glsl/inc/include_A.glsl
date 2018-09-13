// This is shader A for include test

#include "inc/include_B.glsl"
#include "inc/include_C.glsl"
#include "inc/include_D.glsl"

/*
#include "inc/include_F.glsl"
*/

// text after block comment above is still processed

void IncludeTestA()
{
    // This comment should be expanded

}