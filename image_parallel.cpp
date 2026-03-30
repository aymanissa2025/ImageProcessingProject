#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
using namespace std;

struct Pixel {
    int r, g, b;
};

int main() {
    ifstream in("input.ppm");// for reading from the file
    ofstream out("output_parallel.ppm");// then writing the results we get to a new file
   
    if (!in) {
        cout << "Error: could not open input.ppm\n";
        return 1;
    }
    // here we are reading the format of the image and its details 
    string format;
    int width, height, maxval;// maxval is the max number for the values of rgb

    in >> format >> width >> height >> maxval;

        if (format != "P3") {
        cout << "Error: only P3 PPM format is supported.\n";
        return 1;
    }
    vector<vector<Pixel>> image(height, vector<Pixel>(width));// we use these for storing the images 
    vector<vector<Pixel>> grayImage(height, vector<Pixel>(width));
    vector<vector<Pixel>> blurImage(height, vector<Pixel>(width));// here like the serial we are using the 3x3 average filter

    // Reading the values of the pixel in the image 
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            in >> image[i][j].r >> image[i][j].g >> image[i][j].b;
        }
    }
    
    double start = omp_get_wtime();// for the timing

    int totalGray = 0;// where we accumulate the sum of all grayscale values that we obtain
    // which at the end can tell us the brightness of the image showing that the computation worked 

   #pragma omp parallel shared(image, grayImage, blurImage, width, height) reduction(+:totalGray) // shared(): we are using these values across all threads and we used reduction
    {
        //we used reduction so that each thread will be able to compute it's own value and then at the end they will be added together
        #pragma omp single// single means that only one thread will execute this block and print for us the number of threads that were used 
        {
            cout << "Running with " << omp_get_num_threads() << " threads\n";
        }

        // 
        #pragma omp for collapse(2) schedule(static)//collapse(2) means that nested loops will be parallelized together and scheduele(static) that we have iterations for threads are evenly divided
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {

                int gray = (image[i][j].r + image[i][j].g + image[i][j].b) / 3;// to compute average for grayscale like before

                grayImage[i][j].r = gray;
                grayImage[i][j].g = gray;
                grayImage[i][j].b = gray;

                totalGray += gray;
            }
        }
          #pragma omp single
        {
            blurImage = grayImage;
        }

        // this is for the 3x3 blur filter 
        #pragma omp for schedule(dynamic)//schedule(dynamic) we used it to balance the workload aming threads
        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {

                int sum = 0;

                for (int x = -1; x <= 1; x++) {
                    for (int y = -1; y <= 1; y++) {
                        sum += grayImage[i + x][j + y].r;
                    }
                }// same work as before for blurring

                int avg = sum / 9;

                blurImage[i][j].r = avg;
                blurImage[i][j].g = avg;
                blurImage[i][j].b = avg;
            }
        }

        // here we are also ensuring that one thread prints at a time
        #pragma omp critical
        {
            cout << "Thread " << omp_get_thread_num() << " finished\n";// which one has finished 
        }
    }

    double end = omp_get_wtime();

    // Write output of the image to output file
    out << format << "\n" << width << " " << height << "\n" << maxval << "\n";

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            out << blurImage[i][j].r << " "
                << blurImage[i][j].g << " "
                << blurImage[i][j].b << " ";
        }
        out << "\n";
    }

    cout << "Total gray sum: " << totalGray << endl;
    cout << "Parallel time: " << (end - start) << " seconds\n";

    in.close();
    out.close();

    return 0;
}