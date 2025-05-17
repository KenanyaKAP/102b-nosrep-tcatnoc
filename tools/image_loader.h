#include <MiniDNN.h>

using namespace MiniDNN;

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#define CHANNEL_NUM 1
#define THRESHOLD 128

// Function to load an image, convert to binary, crop, resize to 30x40, and add 5px white border
void load_img(int num, int index, const char* format_image, uint8_t** image, int* width, int* height) {
    Eigen::MatrixXi numM(1, 7);
    numM << 9, 8, 7, 5, 4, 2, 0;

    int original_width, original_height, bpp;

    // Load the image
    std::string image_path = "number/" + std::to_string(numM(0, num-index)) + format_image;
    uint8_t* original_image = stbi_load(image_path.c_str(), &original_width, &original_height, &bpp, CHANNEL_NUM);
    if (!original_image) {
        printf("Error loading image!\n");
        throw std::runtime_error("Belum ada gambar angka di folder number :(");

        *image = NULL;
        *width = 0;
        *height = 0;
        return;
    }

    // Convert to binary (0 or 255)
    for (int i = 0; i < original_width * original_height * CHANNEL_NUM; i++) {
        original_image[i] = (original_image[i] > THRESHOLD) ? 255 : 0;
    }

    // Find the bounding box of non-white pixels
    int min_x = original_width;
    int min_y = original_height;
    int max_x = 0;
    int max_y = 0;

    for (int y = 0; y < original_height; y++) {
        for (int x = 0; x < original_width; x++) {
            int idx = y * original_width + x;
            if (original_image[idx] == 0) {  // If pixel is black
                if (x < min_x) min_x = x;
                if (y < min_y) min_y = y;
                if (x > max_x) max_x = x;
                if (y > max_y) max_y = y;
            }
        }
    }

    // Ensure we found some black pixels
    if (min_x > max_x || min_y > max_y) {
        printf("No black pixels found in the image\n");
        stbi_image_free(original_image);
        *image = NULL;
        *width = 0;
        *height = 0;
        return;
    }

    // Calculate dimensions of cropped image
    int crop_width = max_x - min_x + 1;
    int crop_height = max_y - min_y + 1;

    // Create a new image buffer for the cropped image
    uint8_t* cropped_image = (uint8_t*)malloc(crop_width * crop_height * CHANNEL_NUM);
    if (!cropped_image) {
        printf("Error allocating memory for cropped image\n");
        stbi_image_free(original_image);
        *image = NULL;
        *width = 0;
        *height = 0;
        return;
    }

    // Copy the cropped part of the image
    for (int y = 0; y < crop_height; y++) {
        for (int x = 0; x < crop_width; x++) {
            int src_idx = (y + min_y) * original_width + (x + min_x);
            int dst_idx = y * crop_width + x;
            cropped_image[dst_idx] = original_image[src_idx];
        }
    }

    // Create a new buffer for the resized image (30x40)
    const int target_width = 16;
    const int target_height = 26;
    uint8_t* resized_image = (uint8_t*)malloc(target_width * target_height * CHANNEL_NUM);
    if (!resized_image) {
        printf("Error allocating memory for resized image\n");
        free(cropped_image);
        stbi_image_free(original_image);
        *image = NULL;
        *width = 0;
        *height = 0;
        return;
    }

    // Resize the cropped image to 30x40
    stbir_resize_uint8_linear(
        cropped_image, crop_width, crop_height, 0,
        resized_image, target_width, target_height, 0,
        (stbir_pixel_layout)CHANNEL_NUM
    );

    // Create final image with white border (40x50)
    const int border = 2;
    const int final_width = target_width + 2 * border;
    const int final_height = target_height + 2 * border;
    uint8_t* final_image = (uint8_t*)malloc(final_width * final_height * CHANNEL_NUM);
    if (!final_image) {
        printf("Error allocating memory for final image\n");
        free(resized_image);
        free(cropped_image);
        stbi_image_free(original_image);
        *image = NULL;
        *width = 0;
        *height = 0;
        return;
    }

    // Fill the entire image with white pixels (255)
    for (int i = 0; i < final_width * final_height * CHANNEL_NUM; i++) {
        final_image[i] = 255;
    }

    // Copy the resized image into the center of the final image
    for (int y = 0; y < target_height; y++) {
        for (int x = 0; x < target_width; x++) {
            int src_idx = y * target_width + x;
            int dst_idx = (y + border) * final_width + (x + border);
            final_image[dst_idx] = resized_image[src_idx];
        }
    }

    // Free intermediate buffers
    free(resized_image);
    free(cropped_image);
    stbi_image_free(original_image);

    // Set the output parameters
    *image = final_image;
    *width = final_width;
    *height = final_height;
}