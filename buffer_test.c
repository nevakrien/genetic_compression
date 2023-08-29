#include "file_buffer.c"

bool filesAreEqual(const char* file1, const char* file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return false;
    }

    bool isEqual = true;
    unsigned char byte1, byte2;

    while (isEqual && (fread(&byte1, 1, 1, f1) > 0) && (fread(&byte2, 1, 1, f2) > 0)) {
        if (byte1 != byte2) {
            isEqual = false;
        }
    }

    fclose(f1);
    fclose(f2);
    return isEqual;
}

int main() {
    const char* inputFilename = "test_garbage/input.dat";
    const char* outputFilename = "test_garbage/output.dat";

    LinkedList list = { NULL, NULL, 0, 0 };

    printf("Attempting to read from %s...\n", inputFilename);
    if (!readBytesFromFile(inputFilename, &list)) {
        printf("Error reading from file %s!\n", inputFilename);
        return 1;
    }

    printf("Attempting to write to %s...\n", outputFilename);
    if (!writeBytesToFile(outputFilename, &list)) {
        printf("Error writing to file %s!\n", outputFilename);
        return 1;
    }

    if (filesAreEqual(inputFilename, outputFilename)) {
        printf("Both files are identical!\n");
    } else {
        printf("Files are different!\n");
    }

    // Delete the output file after checking
    printf("Removing file %s...\n", outputFilename);
    remove(outputFilename);

    // Cleanup LinkedList to free allocated memory
    cleanupLinkedList(&list);


    return 0;
}

