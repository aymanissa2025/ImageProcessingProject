#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
using namespace std;
struct Pixel{ // here we specified the colors of the pictures which will always be a mix of red green and blue
    int r , g , b;
};

int main() {
    ifstream in("input.ppm");//reads the input which is an image
    ofstream out("output_serial.ppm");// the result at the end

        if (!in) {
        cout << "Error: could not open input.ppm\n";
        return 1;
    }
    
    string format;//the info of the image from the file
    int width, height, maxval;

    in >> format >> width >> height >> maxval;

      if (format != "P3") {
        cout << "Error: only P3 PPM format is supported.\n";
        return 1;
    }
    vector<vector<Pixel>> image(height, vector<Pixel>(width));
    vector<vector<Pixel>> grayImage(height, vector<Pixel>(width));
    vector<vector<Pixel>> blurImage(height, vector<Pixel>(width));

    // Reading the pixels in the image one by one
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            in >> image[i][j].r >> image[i][j].g >> image[i][j].b;
        }
    }
    double start = omp_get_wtime();

    //this function for grayscale turning an image to a different shades of grey
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int gray = (image[i][j].r + image[i][j].g + image[i][j].b) / 3;// over 3 because we have red green and blue so (0,0,0) for example so it divides them to get the gray color

            grayImage[i][j].r = gray;
            grayImage[i][j].g = gray;
            grayImage[i][j].b = gray;
        }// this means that the three colors are now gray of the pixel 
    }
    blurImage = grayImage;
    // This is for the blur function which makes the image softer or less sharp
    // we make each pixel become the average of it's neighbors which becomes the value of the single pixel
    for (int i = 1; i < height - 1; i++) {// we started the loop from 1 to avoid the borders because they don't have neighbors from all sides
        for (int j = 1; j < width - 1; j++) {

            int sum = 0;

            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    sum += grayImage[i + x][j + y].r;
                }
            }

            int avg = sum / 9;// because the average pixel has 9 surrounding direct neighbors including it here we're saying 3x3

            blurImage[i][j].r = avg;
            blurImage[i][j].g = avg;
            blurImage[i][j].b = avg;// to store the results 
        }
    }
    double end = omp_get_wtime();

    // Write the output
    out << format << "\n" << width << " " << height << "\n" << maxval << "\n";// writes the output into a new file that we declared in the beginning 

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            out << blurImage[i][j].r << " "
                << blurImage[i][j].g << " "
                << blurImage[i][j].b << " ";// here we are looping over the image and writing its pixel values 
        }
        out << "\n";
    }

    cout << "Serial processing took: " << (end-start) << " seconds.\n";

    in.close();
    out.close();

    return 0;
}