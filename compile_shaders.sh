#!/bin/bash

#/home/nic/Staging/VulkanSDK/1.3.204.1/x86_64/bin/glslangValidator -V100 -o Draw-vert.spv Draw.vert
#/home/nic/Staging/VulkanSDK/1.3.204.1/x86_64/bin/glslangValidator -V100 -o Draw-frag.spv Draw.frag
#/home/nic/Staging/VulkanSDK/1.3.204.1/x86_64/bin/glslangValidator -V100 -o DrawTex-vert.spv DrawTex.vert
#/home/nic/Staging/VulkanSDK/1.3.204.1/x86_64/bin/glslangValidator -V100 -o DrawTex-frag.spv DrawTex.frag
$VULKAN_SDK/bin/glslangValidator -V100 -o Draw-vert.spv Draw.vert
$VULKAN_SDK/bin/glslangValidator -V100 -o Draw-frag.spv Draw.frag
$VULKAN_SDK/bin/glslangValidator -V100 -o DrawTex-vert.spv DrawTex.vert
$VULKAN_SDK/bin/glslangValidator -V100 -o DrawTex-frag.spv DrawTex.frag