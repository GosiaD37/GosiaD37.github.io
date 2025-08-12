#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

// Function to display the multiplication table
void displayMultiplicationTable(int start, int end) {
    for (int i = start; i <= end; i++) {
        for (int j = start; j <= end; j++) {
            cout << i << " * " << j << " = " << i * j << endl;
        }
    }
}

// Function to save the multiplication table to a file
void saveTableToFile(int start, int end, const string& filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cerr << "Error: Could not open file for writing." << endl;
        return;
    }

    for (int i = start; i <= end; i++) {
        for (int j = start; j <= end; j++) {
            outFile << i << " * " << j << " = " << i * j << endl;
        }
    }

    outFile.close();
    cout << "Multiplication table saved to " << filename << endl;
}

// Function to get a validated integer input from the user
int getValidatedInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (cin.fail()) {
            cout << "Invalid input. Please enter an integer.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            break;
        }
    }
    return value;
}

int main() {
    cout << "=== Custom Multiplication Table Generator ===\n";

    int start = getValidatedInput("Enter the start of the range: ");
    int end = getValidatedInput("Enter the end of the range: ");

    if (start > end) {
        cout << "Start value cannot be greater than end value. Exiting.\n";
        return 1;
    }

    cout << "\nGenerated Multiplication Table:\n";
    displayMultiplicationTable(start, end);

    char saveOption;
    cout << "\nWould you like to save this table to a file? (Y/N): ";
    cin >> saveOption;

    if (saveOption == 'Y' || saveOption == 'y') {
        string filename;
        cout << "Enter filename to save (e.g., table.txt): ";
        cin >> filename;
        saveTableToFile(start, end, filename);
    } else {
        cout << "Table not saved.\n";
    }

    return 0;
}
