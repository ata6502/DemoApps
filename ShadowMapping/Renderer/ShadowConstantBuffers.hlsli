cbuffer CBufferPerFrame : register(b0)
{
    matrix View;
    matrix Projection;
};

cbuffer CBufferPerObject : register(b1)
{
    matrix World;
};



