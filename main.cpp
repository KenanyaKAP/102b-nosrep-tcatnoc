#include <MiniDNN.h>
#include "image_loader.h"

using namespace MiniDNN;

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;

int main()
{
    Eigen::MatrixXd x(1, 11);
    x << 6, 2, 3, 4, 8, 6, 8, 12, 11, 9, 15;

    const int single_width = 20;
    const int height = 30;
    const int total_width = single_width * x.cols(); // 20 * 11 = 220
    uint8_t* combined_image = (uint8_t*)malloc(total_width * height * CHANNEL_NUM);
    if (!combined_image) {
        printf("Error allocating memory for combined image\n");
        return 1;
    }

    memset(combined_image, 255, total_width * height * CHANNEL_NUM);

    // Create Neural Network / AI
    Network net;

    // Layer 1 -- 1 input, 256 output
    Layer* layer1 = new FullyConnected<ReLU>(1, 256);
    // Layer 2 -- 256 input, 512 output
    Layer* layer2 = new FullyConnected<ReLU>(256, 512);
    // Layer 3 -- 512 input, 20*30 output
    Layer* layer3 = new FullyConnected<Tanh>(512, 20 * 30);

    // Add layers to the network object
    net.add_layer(layer1);
    net.add_layer(layer2);
    net.add_layer(layer3);

    // Set output layer
    net.set_output(new RegressionMSE());
    try {
        std::cout << "Mencoba baca network..." << std::endl;
        net.read_net("./NetFolder/", "NetFile");
        std::cout << "Network berhasil dibaca!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Gagal baca network, inisialisasi ulang..." << std::endl;
        net.init(0, 0.01);
    }

    for (int j = 0; j < x.cols(); ++j) {
        double value = x(0, j);
        x(0, j) = (value-j+1)*10;
    }

    Matrix pred = net.predict(x);

    for (int j = 0; j < x.cols(); j++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < single_width; x++) {
                int pred_idx = y * single_width + x;
                
                int img_idx = y * total_width + (j * single_width) + x;
                
                double value = std::max(0.0, std::min(1.0, pred(pred_idx, j)));
                combined_image[img_idx] = static_cast<uint8_t>(value * 255.0);
            }
        }
    }

    std::cout << "Menyimpan nomorku..." << std::endl;
    stbi_write_png("nomorku.png", total_width, height, CHANNEL_NUM, 
                combined_image, total_width * CHANNEL_NUM);

    // Free the memory
    free(combined_image);

    return 0;
}