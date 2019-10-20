//
//  board.h
//  pphint
//
//  Created by Albrecht Eckardt on 2018-07-23.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#ifndef board_h
#define board_h

#include <stdio.h>
#include <iostream>

#include "image/image.h"
#include "image/matchprofile.h"
#include "xml/xmlres.h"

/////////////////////////////////////////////////////////////////////////////
//
// GameEngine
//
/////////////////////////////////////////////////////////////////////////////

std::string categoryToStr(uchar n_cat);
uchar strToCategory(const std::string& str);

#define GAME_COLUMNS 11
#define GAME_ROWS    7
#define BOARD_ITEMS  (GAME_COLUMNS * GAME_ROWS)

class GameEngine
{
public:
    // constructor
    inline GameEngine() { score = 0; multiplier = 0; }

    // core functions
    bool isPossibleMove(uint8_t pos1, uint8_t pos2) const;
    void executeMove(uint8_t pos1, uint8_t pos2);
    void makeCombos();
    
    int getScore() const { return score; }
    void setScore(int value) { score = value; }
    
    void calculateBestMove(int& pos1, int& pos2) const;
    
    // basic access methods
    inline char getItem(int pos) const { return items[pos]; }
    inline short getLevel(int pos) const { return levels[pos]; }
    inline bool isProtected(int pos) const { return protection[pos]; }
    
    inline void setItem(int pos, char item) { items[pos] = item; }
    inline void setLevel(int pos, short level) { levels[pos] = level; }
    inline void setProtection(int pos, bool val) { protection[pos] = val; }
    
    inline uchar getCategory(int pos) const { return strToCategory(getStr(pos)); }
    inline std::string getStr(int pos) const; 
    inline void setCategory(int pos, const uchar n_cat) { setStr(pos, categoryToStr(n_cat)); }
    inline void setStr(int pos, const std::string& str);
    
    inline char getFarmerPos() const { return farmer_pos; }
    inline char getFarmerKO() const { return farmer_ko; }
    inline void setFarmerPos(char pos) { farmer_pos = pos; }
    inline void setFarmerKO(char n_moves) { farmer_ko = n_moves; }
    
    inline char getFarmerDirection() const { return farmer_dir; }
    inline void setFarmerDirection(char dir) { farmer_dir = dir; }
    
    inline const std::string& getFallingItems() const { return falling_items; }
    inline void setFallingItems(std::string& falling) { falling_items = falling; }
    
    // print methods
    void print() const;
    
    GameEngine& operator =(const GameEngine& ge);
    bool operator ==(const GameEngine& ge);
    
private:
    enum ComboType
    {
        THREE,
        FOUR,
        FIVE,
        L_OR_T,
        MORE_THAN_FIVE_STRAIGHT,
        BIG_L_OR_T,
        UNKNOWN
    };
    
    struct Combo
    {
        char color;
        std::vector<uint8_t> items;
        uint8_t center;
        bool protection;
        ComboType type;
        bool modified;
    };
    
    struct MoveValue
    {
        int score;
        int color_value[5];
        int growth[5]; // 5 combo || burning => growth
        int total_flow;
        int total_growth;
        
        int max_score_2nd_it;
        int max_color_value_2nd_it[5];
        int max_growth_2nd_it[5];
        int max_total_flow_2nd_it;
        int max_total_growth_2nd_it;
        
        int max_score_3rd_it;
        int max_color_value_3rd_it[5];
        int max_total_flow_3rd_it;
        int max_total_growth_3rd_it;
    };
    
    char items[BOARD_ITEMS];
    short levels[BOARD_ITEMS];
    bool protection[BOARD_ITEMS];
        
    char farmer_pos;
    char farmer_ko;
    char farmer_dir;
    
    std::string falling_items;
    
    char burning_order[BOARD_ITEMS];
    char total_burning;
    
    int score;
    double multiplier;
    
    char growth[5];

    bool hasMoveH(uint8_t i, uint8_t j) const;
    bool hasMoveV(uint8_t i, uint8_t j) const;
    
    void simulateMoveH(
            uint8_t i, uint8_t j,
            bool& sym1h, bool& sym2h,
            uint8_t &first_sym1v, uint8_t& last_sym1v,
            uint8_t& first_sym2v, uint8_t& last_sym2v) const;
    void simulateMoveV(
            uint8_t i, uint8_t j,
            bool& sym1v, bool& sym2v,
            uint8_t &first_sym1h, uint8_t& last_sym1h,
            uint8_t& first_sym2h, uint8_t& last_sym2h) const;
    
    void addCombo(std::vector<Combo>& combos, uint8_t first, uint8_t last) const;
    void addSwapCombos(std::vector<Combo>& combos, uint8_t pos1, uint8_t pos2) const;
    
    void comboPrint(std::vector<Combo>& combos) const;
    
    void findCombos(std::vector<Combo>& combos) const;
    void linkCombos(std::vector<Combo>& combos) const;
    void handleIntersections(std::vector<Combo>& combos) const;
    void redefineCombos(std::vector<Combo>& combos) const;
    void determineComboTypes(std::vector<Combo>& combos) const;
    
    void waveGrow(int center, bool horizontal);
    void growAll(char color);
    void protectAll(char color);
    
    void reduce(std::vector<Combo>& combos);
    void drop();
    void fill();
    
    void runFarmer();
    
    void resetBurning();
    void checkBurning();
    void burnNext();
    
    void pepperPanic();
    
    int getLevelScore(short level);
    
    void determineColorValue(const char color, int& color_value) const;
    void determineMoveValue(int iteration_level, const std::pair<uint8_t, uint8_t> &move, MoveValue& move_value) const;
    
    void swap(uint8_t i1, uint8_t i2);
};

/////////////////////////////////////////////////////////////////////////////
//
// RecognitionTree struct declaration
//
/////////////////////////////////////////////////////////////////////////////

struct RecognitionTree
{
    struct ItemCategory
    {
        std::vector<size_t> indexes;
        std::vector<ItemCategory> subcategories;
        
        bool use_exclusion;
        bool best_match;
        
        double min_threshold;
        short max_diff;
        
        uint16_t mp_index;
    };
    
    ItemCategory root;
};

/////////////////////////////////////////////////////////////////////////////
//
// DataContainer class declaration
//
/////////////////////////////////////////////////////////////////////////////

class DataContainer : public XMLResource
{
public:
    DataContainer(const char* filename);
    
    // capture board from screen
    bool captureGameRegion(cv::Point& id_pt, Image& rgn, cv::Point& mod) const;
    bool findIdentifier(const Image& screen, cv::Point& id_pt) const;
    
    void translate(const cv::Point& id_pt, cv::Point& mod) const;
   
    // extract item from gameboard
    void extractCell(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j, Image& cell) const;
    void extractFarmer(const Image& rgn, const cv::Point& mod, uint8_t k, Image& farmer) const;
    void extractFarmerKO(const Image& rgn, const cv::Point& mod, uint8_t k, Image& farmer_ko) const;
    
    // identification methods
    int findFarmer(const Image& rgn, const cv::Point& mod) const;
    int identifyItemCategory(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j) const;
    int tryAgainIdentify(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j) const;
    
    // basic access methods
    inline const RecognitionTree& getRecognitionTree() const { return recognition_tree; }
    inline const std::string& getName(size_t index) const { return names[index]; }
    
    inline int getCellWidth() const { return cell_width; }
    inline int getColumnPos(size_t index) const { return column_pos[index]; }
    inline int getRowPos(size_t index) const { return row_pos[index]; }
    
protected:
    void members(Mode mode);
    
private:
    void captureIdentifier(const cv::Point& id_pt, Image& id_rect) const;
    void extractIdentifier(const Image& screen, const cv::Point& id_pt, Image& id_rect) const;

    void captureAll(const cv::Point& id_pt, Image& rgn, cv::Point& mod) const;
    
    int identifyItemCategory(const Image& rgn, const cv::Point& mod, uint8_t i, uint8_t j, const RecognitionTree::ItemCategory& rtic) const;
    
    void loadMatchProfile(RecognitionTree::ItemCategory& rtic, uint16_t &mp_index);
    void loadMatchProfiles();
    
    void recalcDimensions();

private:    
    Image identifier;
    ImageArray identifiers;
    
    cv::Point distance_to_board;
    
    int cell_width;
    int cell_height;
    
    std::vector<int> column_pos;
    std::vector<int> row_pos;
    
    int board_width;
    int board_height;
    
    cv::Point distance_to_farmer;
    
    int farmer_width;
    int farmer_height;
    
    cv::Point distance_to_farmer_ko;
    
    int farmer_ko_width;
    int farmer_ko_height;
    
    cv::Point distance_to_topleft;
    
    int total_width;
    int total_height;
    
    std::vector<std::string> names;
    
    RecognitionTree recognition_tree;
    ImageArray match_profile_images;
    MatchProfileArray match_profiles;
    ImageArray items_comp;
    
    ImageArray farmer_rep;
    ImageArray farmer_ko_rep;
    
    std::string identifier_filename;
    std::string identifier_array_filename;
    std::string items_comp_filename;
    std::string match_profiles_filename;
    std::string farmer_representation_filename;
    std::string farmer_ko_representation_filename;
};

/////////////////////////////////////////////////////////////////////////////
//
// GameEngine inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline std::string GameEngine::getStr(int pos) const
{
    std::string catStr;
    catStr += items[pos];
    catStr += static_cast<char>('0' + levels[pos]);
    catStr += protection[pos] ? '#' : '_';
    return catStr;
}

inline void GameEngine::setStr(int pos, const std::string& str)
{
    if (str.size() != 3)
    {
        items[pos] = '?';
        levels[pos] = 0;
        protection[pos] = false;
    }
    else
    {
        items[pos] = str[0];
        levels[pos] = str[1] - '0';
        protection[pos] = str[2] == '#';
    }
}

#endif /* board_h */
