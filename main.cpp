#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include "BS_thread_pool.hpp"
#include "getUserInput.h"

using namespace std;

const string sameChoiceID = "Same Choice";
const string randomChoiceID = "Random Choice";
const string zigZagChoiceID = "Zig Zag";

int amountOfTests = 0;
int amountOfQuestions = 0;
int amountOfChoicesPerQuestion = 0;
unordered_map<string, unordered_map<int, int>> testResults;
mutex testResultsMutex;

static int getRandomChoice() {
    return 1 + (rand() % (amountOfChoicesPerQuestion - 1 + 1));
}

static void runTest(int test) {
    unordered_map<int, int> questions;

    for (int question = 1; question < amountOfQuestions; ++question) {
        questions[question] = getRandomChoice();
    }

    int randomChoiceCorrect = 0;
    int sameChoice = getRandomChoice();
    int sameChoiceCorrect = 0;
    int sigZagChoice = getRandomChoice();
    int sigZagChoiceCorrect = 0;

    for (int question = 1; question <= amountOfQuestions; ++question) {
        if (sigZagChoice >= amountOfChoicesPerQuestion) sigZagChoice = 0;

        sigZagChoice++;

        int answer = questions[question];
        int randomChoice = getRandomChoice();

        if (answer == randomChoice) randomChoiceCorrect++;
        if (answer == sameChoice) sameChoiceCorrect++;
        if (answer == sigZagChoice) sigZagChoiceCorrect++;
    }

    testResultsMutex.lock();

    testResults[sameChoiceID][test] = sameChoiceCorrect;
    testResults[randomChoiceID][test] = randomChoiceCorrect;
    testResults[zigZagChoiceID][test] = sigZagChoiceCorrect;

    testResultsMutex.unlock();
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    amountOfTests = getUserInput("Amount of tests: ");
    amountOfQuestions = getUserInput("Amount of questions per test: ");
    amountOfChoicesPerQuestion = getUserInput("Amount of choices per question: ");

    cout << "Running tests..." << endl;

    testResults[sameChoiceID] = {};
    testResults[randomChoiceID] = {};
    testResults[zigZagChoiceID] = {};

    BS::thread_pool pool;

    for (int test = 1; test <= amountOfTests; ++test) {
        pool.push_task(runTest, test);
    }

    while (pool.get_tasks_total() > 0) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << endl << "Results:" << endl;

    unordered_map<string, int> results;
    unordered_map<string, double> avgResults;
    results[sameChoiceID] = 0;
    avgResults[sameChoiceID] = 0;
    results[randomChoiceID] = 0;
    avgResults[randomChoiceID] = 0;
    results[zigZagChoiceID] = 0;
    avgResults[zigZagChoiceID] = 0;
    results["ties"] = 0;
    avgResults["ties"] = 0;

    for (int test = 1; test <= amountOfTests; ++test) {
        int highest = 0;
        string highestGuessType;

        for (const auto& guessType : testResults) {
            int score = testResults[guessType.first][test];
            avgResults[guessType.first] += (static_cast<double>(score) / amountOfQuestions) * 100;

            if (score > highest) {
                highest = testResults[guessType.first][test];
                highestGuessType = guessType.first;
            } else if (score == highest) {
                highestGuessType = "ties";
            }
        }

        results[highestGuessType]++;
    }

    for (const auto& guessType : results) {
        cout << guessType.first << ": " << guessType.second << " / " << (avgResults[guessType.first] / amountOfTests) << "% avg" << endl;
    }

    return EXIT_SUCCESS;
}
