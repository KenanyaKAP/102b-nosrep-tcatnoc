#include <MiniDNN.h>
#include "image_loader.h"

using namespace MiniDNN;

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;

// Ubah sesuai kebutuhan
bool reset = false;
int epoch = 500;
double learning_rate = 0.002;
const char* format_image = ".jpeg";

int main()
{
    uint8_t* image = NULL;
    int width, height;
    
    const int imgSize = 20 * 30;
    
    Eigen::MatrixXd x(1, 7);
    Matrix y = Matrix::Zero(imgSize, x.cols());

    x << 6, 6, 6, 6, 6, 6, 6;

    for (int j = 0; j < x.cols(); ++j) {
        double value = x(0, j);

        load_img(value, j, format_image, &image, &width, &height);
        
        if (image) {
            for (int i = 0; i < width * height; i++) {
                y(i, j) = image[i] / 255.0;
            }
                        
            free(image);
            image = NULL;
        } else {
            std::cerr << "Gambarnya belum ada :(" << j << std::endl;
        }
    }

    x << 70, 60, 50, 40, 30, 20, 10;

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

    // Create optimizer object
    RMSProp opt;
    opt.m_lrate = learning_rate;

    VerboseCallback callback;
    net.set_callback(callback);

    // Buat dari awal?
    if (reset) {
        // Initialize parameters
        net.init(0, 0.01);
    } else {
        // Lanjutkan training
        try {
            std::cout << "Mencoba baca network..." << std::endl;
            net.read_net("./NetFolder/", "NetFile");
            std::cout << "Network berhasil dibaca!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Gagal baca network, inisialisasi ulang..." << std::endl;
            net.init(0, 0.01);
        }
    }

    // Train the network
    std::cout << "Training..." << std::endl;
    net.fit(opt, x, y, 100, epoch);
    net.export_net("./NetFolder/", "NetFile");

    return 0;
}