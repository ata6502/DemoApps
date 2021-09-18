cbuffer ConstantBufferOnResize : register(b0)
{
    matrix Projection;
};

cbuffer ConstantBufferPerFrame : register(b1)
{
    matrix View;
};

cbuffer ConstantBufferPerObject : register(b2)
{
    matrix Model;
};
