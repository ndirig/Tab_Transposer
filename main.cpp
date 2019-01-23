
/*
 * -- NOAH'S TAB TRANSPOSER --
 *
 *  Takes a tab inputted via console, transposes it to a new key
 *  (as indicated by the user), and outputs transposed tab to
 *  console.
 *
 *  Copyright Noah Dirig 2019
 *
 */

/*
 * -- Known issues / limitations:
 *   >  Does not transpose correctly when major chords are
 *      labeled with a capital 'M' (ex: AM)
 *   >  Transposes song titles and artists starting with two
 *      of the same letter (ex: 'B B King' -> 'F# F# King')
 *   >  Doesn't work yet with true flat and sharp symbols
 *      (♭, ♯) or superscripts
 *   >  Cannot transpose from a major key to a minor key
 *   >  Doesn't work with 'no5' chords and such
 *   >  Slash chords that are also 6/9 chord (such as
 *      D6/9/A) don't work
 *   >  'Bbmaj7#11/A' transposed from A to F major returns
 *      'F#maj7#11/AF', with an extra 'F'
 *   >  When a song structure tag such as "Intro:" is placed
 *      before chords on the same line, those chords are not
 *      transposed.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <cctype>
#include <algorithm>
#include <functional>

using namespace std;

// sorted dictionaries of valid notes (w/o sharps or flats)
// and chord notations
const vector<string> notes = {"ab","a","bb","b","c","db","d",
                              "eb","e","f","f#","g"};
// alternate notation for the same notes above
const vector<string> altNotes = {"g#","a","a#","b","b#","c#","d",
                                 "d#","e","e#","gb","g"};
vector<string> chords = {"#5#9","#5b9","11","13","13#11",
                         "13sus", "13sus2","13sus4","2","5","6",
                         "6/9","7", "7#11","7#5","7#9","7b5",
                         "7b5#9", "7b5(#9)","7b9","7sus","7sus2",
                         "7sus4", "9","9sus","9sus2","9sus4","m",
                         "add9","aug","aug7#9","aug9","b5","b5#9",
                         "b5b9","dim","dim7","m","m(add9)",
                         "m(maj7)","m11","m13","m6","m6/9", "m7",
                         "m7b5","m7b9","m9","m9(maj7)","m9m7",
                         "m9b5","m9maj7","mm7","madd9","maj",
                         "maj13","maj7","maj7#11","maj9","major",
                         "mb6","min","minor","mmaj7","sus",
                         "sus2","sus4"};

// method prototype
int getNoteIndex(const vector<string>& chromatic, string note);
bool isAltNote(string note);
bool validChord(string word);
bool isFlatSharpFive(string& word, int& flat5Ind);
bool validSlashChord(string& word);

/**
 * Key object for storing data members such as the name, the
 * corresponding index in the note dictionaries, and whether
 * key name can be found in the regular note dictionary or
 * the alternative notation dictionary.
 */
class Key {
private:
    string keyName;
    // internal id used for calculating intervals
    int id;
    // boolean to keep track of whether the name of this
    //   key is found in the altNotes dictionary or not
    bool alt;
public:
    // constructors
    Key() { keyName = "";  id = 0;  alt = false; }

    Key(string keyName) {
        this->keyName = keyName;
        // get a numerical id for key -- this is the index of
        //   the key from a note dictionary (notes or altNotes)
        // if key is labeled w/ alternate notation, use altNotes
        if (isAltNote(keyName)) {
            alt = true;
            id = getNoteIndex(altNotes, keyName);
        } else {
            alt = false;
            id = getNoteIndex(notes, keyName);
        }

    }

    // getters, setter, and to_string
    int keyIndex() { return this-> id; }
    bool usesAltNotes() { return alt; }
    void setKey(string keyName) { this->keyName = keyName; }
    string getKeyName() { return keyName; }
    string to_string() {
        // convert root note to uppercase
        transform(keyName.begin(), ++keyName.begin(),
                  keyName.begin(), ::toupper);
        return keyName + ", " +std::to_string(id);
    }
};  // end Key

/**
 * Determines whether the particular labeling of a note is in
 * the altNotes dictionary (e.g. 'A#' is an alternate notation
 * of the more common 'Bb' and is therefore in the altNotes
 * dictionary)
 * @param note a note name
 * @return where the note name is an alternate notation
 */
bool isAltNote(string note) {
    return note == "g#" || note == "a#" ||
           note == "c#" || note == "d#" ||
           note == "gb";
} // end isAltNote

/**
 * Removes blank spaces from a string and converts to lowercase
 * @param input input note string
 * @return formatted note string
 */
string formatNote(string& input) {
    // convert note string to stringstream to easily ignore
    // extraneous blank spaces
    stringstream stream(input);
    string extractedNote;
    stream >> extractedNote;
    // converts note to lowercase for ease of processing
    transform(extractedNote.begin(), extractedNote.end(),
              extractedNote.begin(), ::tolower);
    return extractedNote;
} // end formatNote

/**
 * Takes a note and checks to see if it is valid
 * Must be within range A-G and may be sharp or flat
 * @param note
 * @return whether the note is a valid note
 */
bool validNote(string& note) {
    // convert note string to stringstream to easily ignore
    // extraneous blank spaces
    stringstream stream(note);
    string extractedNote;
    stream >> extractedNote;
    // converts note to lowercase for ease of processing
    transform(extractedNote.begin(), extractedNote.end(),
              extractedNote.begin(), ::tolower);
    // checks in primary note dictionary
    for (auto n : notes) {
        if (extractedNote == n) { return true; }
    }
    // checks in alternate note dictionary (e.g. if the key
    // of Bb is labeled 'A#' in the original tab
    for (auto n : altNotes) {
        if (extractedNote == n) { return true; }
    }
    return false;
} // end validNote

/**
 * Gets a note's corresponding index in the note dictionaries
 * @param chromatic note dictionary (either notes or altNotes)
 * @param note note name
 * @return index
 */
int getNoteIndex(const vector<string>& chromatic, string note) {
    note = formatNote(note);
    for (unsigned int i = 0; i < chromatic.size(); i++) {
        if (chromatic.at(i) == note) {
            return i;
        }
    }
    return 0;
} // end getNoteIndex

/**
 * Gets the interval between the two keys
 * @param old old key
 * @param newKey new key
 * @return the interval between the two keys
 */
int getInterval(Key old, Key newKey) {
    int interval = newKey.keyIndex() - old.keyIndex();
    if (interval < 0) { interval = 12 + interval; }
    return interval;
} // end getInterval

/**
 * Takes a word that may or may not be a valid chord and returns
 * its "root note"
 * @param word a word that may or may not be a valid chord
 * @return the root note of the word
 */
string getRoot(string& word) {
    int flat5Ind = -1;
    // first check if edge cases like 'b5#9' chord names
    // are found at index 1 or 2
    if (isFlatSharpFive(word, flat5Ind)) {
        // Gb5#9 - root: G (root will be assigned in else)
        // Gbb5#9 - root: Gb (root assigned here)
        if (flat5Ind == 2) {
            return word.substr(0,2);
        }
    } else if (word.find("b") == 1 ||
               word.find("#") == 1) {  // if root is sharp or flat
        return word.substr(0,2);
    }  // assume single letter
    return word.substr(0,1);
} // end getRoot

/**
 * Takes a note and transposes it based on the interval
 * @param old old note
 * @param interval interval
 * @param alt whether altNotes dictionary is being used
 * @return transposed note
 */
string transposeNote(string& old, int interval, bool alt) {
    // find the dictionary used to annotate the old note
    if (isAltNote(old)) {
        if (alt) {
            return altNotes.at((getNoteIndex(altNotes, old) +
                                interval) % 12);
        }
        return notes.at((getNoteIndex(altNotes, old) + interval)
                        % 12);
    }
    else {
        if (alt) {
            return altNotes.at((getNoteIndex(notes, old) +
                                interval) % 12);
        }
        return notes.at((getNoteIndex(notes, old) + interval)
                        % 12);
    }
} // end transposeNotes

/**
 * Takes a valid chord and transposes it
 * @param chord
 * @return
 */
string transposeChord(string& chord, Key& oldKey, Key& newKey) {
    string tranRoot, root = getRoot(chord);
    int interval = getInterval(oldKey, newKey);
    // need to find whether or not the name of the key
    // is an alternate notation
    string newKeyName = newKey.getKeyName();
    bool alt = isAltNote(newKeyName);
    // get new root    CAPITALIZE HERE
    tranRoot = transposeNote(root, interval, alt);
    // convert root note to uppercase
    transform(tranRoot.begin(), ++tranRoot.begin(),
              tranRoot.begin(), ::toupper);
    if (validSlashChord(chord)) {
        // split word into pre-slash str and post-slash str
        int slashInd = chord.find("/");
        string preSlQuality = chord.substr(root.size(),
                                           slashInd);
        string post = chord.substr(slashInd + 1);
        string newPostSl = transposeNote(post, interval, alt);
        // convert bass note to uppercase
        transform(newPostSl.begin(), ++newPostSl.begin(),
                  newPostSl.begin(), ::toupper);
        return tranRoot + preSlQuality + newPostSl;
    }
    // chord quality does not change during tranposition
    string qual = chord.substr(root.size());
    return tranRoot + qual;
} // end transposeChord

/**
 * Checks to see if a word is a b5 or #5 chord (such as 'b5#9')
 * If it is, gets the string index of the b5 or #5
 * @param word word that may or may not be a valid chord
 * @param flat5Ind string index of b5 or #5 chord name
 * @return whether word is a b5 or #5 chord
 */
bool isFlatSharpFive(string& word, int& flat5Ind) {
    vector<string> shFl5 = {"b5#9", "b5b9", "#5b9", "#5#9"};
    for (auto chord : shFl5) {
        if (word.find(chord) != string::npos) {
            flat5Ind = word.find(chord);
            return true;
        }
    }
    return false;
} // end isFlatSharpFive

/**
 * Determine whether a word is a valid slash chord
 * @param word word that may or may not be a valid chord
 * @return whether a word is a valid slash chord
 */
bool validSlashChord(string& word) {
    if (word.find("/") == string::npos) { return false; }
    // not valid if there is more than one slash
    if (word.find_first_of("/") != word.find_last_of("/")) {
        return false;
    }
    int slashInd = word.find("/");
    // not valid if slash is last character in word
    if (slashInd == word.size() - 1) { return false; }
    // split word into pre-slash str and post-slash str
    string preSl = word.substr(0, slashInd);
    string postSl = word.substr(slashInd + 1);
    // check if pre-slash str is a valid chord, then check
    // to see if post-slash str is a valid note
    if (validChord(preSl) && validNote(postSl)) { return true; }
    return false;
} // end validSlashChord

/**
 * Checks whether or not a chord quality / postfix is valid
 * @param qual word that may or may not be a chord quality
 * @return whether or not a chord quality is valid
 */
bool validChordQuality(string qual) {
    // convert to lowercase for ease of processing
    qual = formatNote(qual);
    // use binary search to quickly find quality
    return binary_search(chords.begin(), chords.end(), qual);
} // end validChordQuality

/**
 * Determines whether or not a word is a valid chord
 * @param word word that may or may not be a valid chord
 * @return whether or not a word is a valid chord
 */
bool validChord(string word) {
    // if word is one letter, check if letter is a valid chord
    if (word.size() == 1) {
        if (validNote(word)) { return true; }
        return false;
    }
    // if word contains a slash, it may be a slash chord
    if (validSlashChord(word)) { return true; }
    //   if word contains slash but isn't valid slash chord,
    //   it may still be a 6/9 chord

    // get the root note
    string root = getRoot(word);
    // check to see if root note is valid
    if (!validNote(root)) { return false; }
    // if the valid root has no postfix (if it is a major chord)
    if (root == word) { return true; }
    // get isolated chord quality / postfix
    string qual = word.substr(root.size());
    return validChordQuality(qual);
} // end validChord

/**
 * Determines whether a line of text is comprised of chords.
 * In other words, checks if the line is a chord line
 * @param line line of text
 * @return Whether the line is a chord line
 */
bool isChordLine(string line) {
    stringstream stream(line);
    string word;
    stream >> word;
    /* -- Checks the first TWO words for the following case:
     * Ex: "A Movie Script Ending" by Death Cab - Key: A
     *     First word 'A' is technically a valid chord
     *     'Movie' is not a chord, however, so skip line
     * Ex: " C         F   G7   "
     *     'C' and 'F' are valid chords, so process this line
     */
    // check to see if first word is a valid chord
    if (!validChord(word)) { return false; }
    // then check second word is a valid chord
    stream >> word;
    if (!validChord(word)) { return false; }
    // two valid chords = valid chord line
    return true;
} // end isChordLine

/**
 * Process tab line-by-line and transpose all chords into new key
 * @param tab
 * @return
 */
string transposeTab(string& tab, Key& oldKey, Key& newKey) {
    string line, word, newTab = "";
    stringstream stream(tab);
    // process line-by-line
    while (getline(stream, line)) {
        int strInd = 0;
        // if the line of text is not comprised of chords,
        //   skip this line and move to the next
        if (isChordLine(line)) {
            // while words string chord extract
            stringstream lnStream(line);
            while (lnStream >> word) {  // transpose
                if (validChord(word)) {
                    // string index increments to the position of
                    // the end of the last-transposed chord
                    // -- makes it so that it doesn't transpose
                    // the same chord twice
                    int chordInd = line.find(word, strInd);
                    string newChord =
                            transposeChord(word, oldKey, newKey);
                    line.replace(chordInd, word.size(), newChord);
                    strInd += newChord.size();
                }
            }
        }
        newTab += line + "\n";
    }
    return newTab;
} // end transposeTab

/**
 * Interacts with user, asks for the key of the old tab and what
 * key he/she wants to transpose to
 * @param oldKey an empty key object that will be initialized
 * @param newKey an empty key object that will be initialized
 */
void userIOKeys(Key& oldKey, Key& newKey) {
    cout << "Welcome to Noah's Tab Transposer.  What is "
            "the tonic note in the \noriginal key?  (Ex: for"
            " the key of A minor you would type 'A')";
    bool invalid = true;
    string response;
    // loop until valid key is inputted
    while (invalid) {
        cout << "\n> ";
        cin >> response;
        response = formatNote(response);
        if (validNote(response)) { invalid = false; }
    }
    oldKey = Key(response);
    cout << "\nWhat is the tonic note in the key you would "
            "like to transpose to?";
    invalid = true;
    // loop until valid key is inputted
    while (invalid) {
        cout << "\n> ";
        cin >> response;
        response = formatNote(response);
        if (validNote(response)) { invalid = false; }
    }
    newKey = Key(response);
} // end userIOKeys

/**
 * Interacts with user, asks for contents of tab document
 * @param oldKey the key of the old tab
 * @param newKey the key of the new tab
 */
void userIOTab(Key& oldKey, Key& newKey) {
    string response, tab;
    cout << "\nGreat, now paste the original tab below and type "
            "the word \"end\".\n(You can use control+V on "
            "Windows or command+V on Mac to paste.)" << endl;
    // user enters as much as they want and then types "end"
    while (getline(cin, response)) {
        if (response == "end") { break; }
        tab += response + "\n";
    }
    string newTab = transposeTab(tab, oldKey, newKey);
    cout << "\n\n~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/"
            "~/~/~/~/~/~/~/~/~\n\n  "
            "Here is your transposed tab!  Copy and paste the "
            "text below\n  and you are ret2go.  (You can use "
            "control+C on Windows or \n"
            "  command+C on Mac to copy.)\n\n"
            "~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/~/"
            "~/~/~/~/~/~/~/~\n\n" << newTab << endl;
} // end userIOTab

int main() {
    std::sort(chords.begin(), chords.end());
    Key oldKey, newKey;
    userIOKeys(oldKey, newKey);
    userIOTab(oldKey, newKey);
    return 0;
} // end main
