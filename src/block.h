// block.h
#pragma once

struct Block {
    enum class BlockType { Bedrock, Air, Dirt, Sun, Wood, Leaf, Grass, Sand};
    enum BlockTexture : int {
        GRASS_TOP = 0,
        GRASS_SIDE = 1,
        GRASS_BOTTOM = 2,
        WOOD = 3,
        LEAF = 4,
        SAND = 5,
        WOOD_TOP = 6,
    };
    BlockType type;
    BlockTexture top;
    BlockTexture side;
    BlockTexture bottom;
};

