cbuffer ConstantBufferPerFrame : register(b0)
{
    matrix ViewProj;
};

cbuffer ConstantBufferPerObject : register(b1)
{
    matrix World;
};
