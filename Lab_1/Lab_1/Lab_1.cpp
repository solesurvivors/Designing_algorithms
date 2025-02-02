#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cstdio>

#define CHUNK_SIZE 4'000'000

class DirectOuterSort {
private:
    long _iterations, _segments;

public:
    DirectOuterSort() : _iterations(1), _segments(1) {}

    void SplitToFiles(const std::string& inputFile) {
        _segments = 1;
        std::ifstream fileA(inputFile, std::ios::binary);

        std::ofstream fileB("B.bin", std::ios::binary | std::ios::trunc); 
        std::ofstream fileC("C.bin", std::ios::binary | std::ios::trunc);

        std::string currentRecord;
        bool flag = true;
        int counter = 0;

        while (std::getline(fileA, currentRecord)) {
            currentRecord += "\n";

            if (counter == _iterations) {
                counter = 0;
                flag = !flag;
                ++_segments;
            }

            if (flag) {
                fileB.write(currentRecord.c_str(), currentRecord.size());
            } else {
                fileC.write(currentRecord.c_str(), currentRecord.size());
            }
            ++counter;
        }
        fileA.close();
        fileB.close();
        fileC.close();
    }

    std::string MergePairs() {
        std::string fileA = "A.bin";
        std::ofstream writerA(fileA, std::ios::binary | std::ios::trunc);
        std::ifstream readerB("B.bin", std::ios::binary);
        std::ifstream readerC("C.bin", std::ios::binary);

        std::string elementB, elementC;
        bool hasMoreB = static_cast<bool>(std::getline(readerB, elementB));
        bool hasMoreC = static_cast<bool>(std::getline(readerC, elementC));

        int counterB = 0, counterC = 0;

        while (hasMoreB || hasMoreC) {
            bool useB = false;
            std::string currentRecord;

            if (!hasMoreB || counterB == _iterations) {
                currentRecord = elementC;
            } else if (!hasMoreC || counterC == _iterations) {
                currentRecord = elementB;
                useB = true;
            } else {
                if (std::stoi(elementB) < std::stoi(elementC)) {
                    currentRecord = elementB;
                    useB = true;
                } else {
                    currentRecord = elementC;
                }
            }

            currentRecord += "\n";
            writerA.write(currentRecord.c_str(), currentRecord.size());

            if (useB) {
                hasMoreB = static_cast<bool>(std::getline(readerB, elementB));
                ++counterB;
            } else {
                hasMoreC = static_cast<bool>(std::getline(readerC, elementC));
                ++counterC;
            }

            if (counterB == _iterations && counterC == _iterations) {
                counterB = counterC = 0;
            }
        }
        writerA.close();
        readerB.close();
        readerC.close();

        std::ofstream fileB("B.bin", std::ios::binary | std::ios::trunc);
        std::ofstream fileC("C.bin", std::ios::binary | std::ios::trunc);
        fileB.close();
        fileC.close();

        _iterations *= 2;
        return fileA;
    }

    void Sort(const std::string& inputFile, const std::string& sorted) {
        std::string fileA = inputFile;
        while (true) {
            SplitToFiles(fileA);
            if (_segments == 1) break;
            fileA = MergePairs();
        }
        std::ifstream sortedFile(fileA, std::ios::binary);
        std::ofstream outputFile(sorted, std::ios::trunc);
        std::string line;
        while (std::getline(sortedFile, line)) {
            outputFile << line;
        }
        sortedFile.close();
        outputFile.close();
        std::remove("A.bin");
        std::remove("B.bin");
        std::remove("C.bin");
    }
};

class ModifiedOuterSort {
private:
    long _segments;
    long _iterations;
    long chunk_length;

public:
    ModifiedOuterSort() : _segments(1), _iterations(CHUNK_SIZE), chunk_length(CHUNK_SIZE) {}
    ModifiedOuterSort(int cl) : _segments(1), _iterations(cl), chunk_length(cl) {}

    void ConvertStringToInt(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream fileA(inputFile, std::ios::in);
        std::ofstream fileB(outputFile, std::ios::binary | std::ios::trunc);
        std::string currentRecord;
        int current_length{ 0 };
        std::vector<int> chunk;
        chunk.reserve(chunk_length);
        while (std::getline(fileA, currentRecord)) {
            chunk.push_back(stoi(currentRecord));
            current_length++;
            if (current_length == chunk_length) {
                fileB.write((char*)chunk.data(), sizeof(int) * chunk.size());
                current_length = 0;
                chunk.clear();
            }
        }
        if (current_length > 0) {
            fileB.write((char*)chunk.data(), sizeof(int) * chunk.size());
            current_length = 0;
            chunk.clear();
        }
        fileB.close();
        fileA.close();

    }

    void Preparation(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream fileA(inputFile, std::ios::binary | std::ios::in);
        std::ofstream fileB(outputFile, std::ios::binary | std::ios::trunc);
        std::vector<int> chunk;
        chunk.reserve(chunk_length);
        chunk.resize(chunk_length);
        int c = 0;
        while (1) {
            fileA.read((char*)chunk.data(), sizeof(int) * chunk_length);
            c = fileA.gcount() / 4;
            chunk.resize(c);
            if (c == 0) {
                break;
            }
            if (c > 0) {
                std::sort(chunk.begin(), chunk.end());
                fileB.write((char*) chunk.data(), sizeof(int) * c);
            }
        }
        if (c > 0) {
            std::sort(chunk.begin(), chunk.end());
            fileB.write((char*)chunk.data(), sizeof(int) * c);
        }
        fileB.close();
        fileA.close();
    }

    void SplitToFiles(const std::string& inputFile) {
        _segments = 1;
        std::ifstream fileA(inputFile, std::ios::binary);

        std::ofstream fileB("B.bin", std::ios::binary | std::ios::trunc);
        std::ofstream fileC("C.bin", std::ios::binary | std::ios::trunc);

        std::string currentRecord;
        bool flag = true;
        int counter = 0;
        std::vector<int> v;
        v.reserve(chunk_length);
        while (1) {
            fileA.read((char*)v.data(), sizeof(int) * chunk_length);
            int c = fileA.gcount() / 4;
            if (c == 0) {
                break;
            }
            if (counter == _iterations) {
                counter = 0;
                flag = !flag;
                ++_segments;
            }

            if (flag) {
                fileB.write((char*)v.data(), sizeof(int)*c);
            } else {
                fileC.write((char*)v.data(), sizeof(int) * c);
            }
            counter += c;
        }
        fileA.close();
        fileB.close();
        fileC.close();
    }

    std::string MergePairs() {
        std::string fileA = "A.bin";
        std::ofstream writerA(fileA, std::ios::binary | std::ios::trunc);
        std::ifstream readerB("B.bin", std::ios::binary);
        std::ifstream readerC("C.bin", std::ios::binary);

        std::vector<int> va;
        va.reserve(chunk_length);
        int elementB, elementC;
        readerB.read((char*)&elementB, sizeof(int));
        readerC.read((char*)&elementC, sizeof(int));
        
        int counterB = 0, counterC = 0;
        //int cb = 0;
        //int cc = 0;

        while (!readerB.eof() || !readerC.eof()) {
            bool useB = false;
            int currentRecord;

            if (readerB.eof() || counterB == _iterations) {
                currentRecord = elementC;
            } else if (readerC.eof() || counterC == _iterations) {
                currentRecord = elementB;
                useB = true;
            } else {
                if (elementB < elementC) {
                    currentRecord = elementB;
                    useB = true;
                } else {
                    currentRecord = elementC;
                    if (currentRecord == 0) {
                        int f = 0;
                    }

                }
            }
            va.push_back(currentRecord);
            if (va.size() == chunk_length) {
                writerA.write((char*)va.data(), sizeof(int) * va.size());
                va.clear();
            }
            if (useB) {
                readerB.read((char*)&elementB, sizeof(int));
                //++cb;
                ++counterB;
            } else {
                readerC.read((char*)&elementC, sizeof(int));
                //++cc;
                ++counterC;
            }

            if (counterB == _iterations && counterC == _iterations) {
                counterB = counterC = 0;
            }
        }
        if (va.size() > 0) {
            writerA.write((char*)va.data(), sizeof(int) * va.size());
            va.clear();
        }
        writerA.close();
        readerB.close();
        readerC.close();

        std::ofstream fileB("B.bin", std::ios::binary | std::ios::trunc);
        std::ofstream fileC("C.bin", std::ios::binary | std::ios::trunc);
        fileB.close();
        fileC.close();

        _iterations *= 2;
        //std::cout << cb << " " << cc << "\n";
        return fileA;
    }

    void PostWrite(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream s1(inputFile, std::ios::binary | std::ios::in);
        std::ofstream s2(outputFile, std::ios::trunc);
        std::vector<int> v;
        v.reserve(chunk_length);
        int i = 0;
        while (1) {
            s1.read((char*)&i, sizeof(int));
            s1.read((char*)v.data(), sizeof(int) * chunk_length);
            i = s1.gcount() / 4;
            if (i == 0) {
                break;
            }
            for (int j = 0; j < i; ++j) {
                s2 << v[j] << "\n";
            }
            v.clear();
        }
        if (i > 0) {
            for (int j = 0; j < i; ++j) {
                s2 << v[j] << "\n";
            }
        }
        s1.close();
        s2.close();
        std::remove("A.bin");
        std::remove("B.bin");
        std::remove("C.bin");
    }

    void Sort(const std::string& inputFile) {
        std::string fileA = inputFile;
        while (true) {
            SplitToFiles(fileA);
            if (_segments == 1) break;
            fileA = MergePairs();
        }

    }
};

int main() {
    std::string fileName;

    std::cout << "Enter the file name to sort: ";
    std::cin >> fileName;
    std::string sorted_file_name = "sorted.txt";
    std::cout << "Choose sorting method:\n1. Original External Sorting\n2. Modified External Sorting\n";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
        DirectOuterSort sorter;
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Start external sorting\n";
        sorter.Sort(fileName, sorted_file_name);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Sorting completed in " << duration.count() << " seconds.\n";
    } else if (choice == 2) {
        ModifiedOuterSort temp(CHUNK_SIZE);
        std::cout << "Converting txt file to bin\n";
        auto start = std::chrono::high_resolution_clock::now();
        temp.ConvertStringToInt(fileName, "B.bin");
        std::cout << "Presorting series\n";
        temp.Preparation("B.bin", "A.bin");
        std::cout << "Start external sorting\n";
        temp.Sort("A.bin");
        std::cout << "Converting output bin file to txt\n";
        temp.PostWrite("A.bin", "sorted.txt");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Sorting completed in " << duration.count() << " seconds.\n";
    } else {
        std::cout << "Invalid choice.\n";
    }
    return 0;
}