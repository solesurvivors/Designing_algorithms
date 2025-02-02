#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <utility>
#include <unordered_set>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <thread>

using point = std::pair<uint8_t, uint8_t>;
using chessboard_type = std::array<uint8_t, 64>;
using state_type = uint32_t;

struct results_IDS_tag {
    uint64_t iterations = 0;
    uint64_t nodes = 0;
    size_t memory = 0;
    size_t dead_ends = 0;
    bool success = false;
    std::array<uint8_t, 8> queens1d{};
};

std::ostream& operator<<(std::ostream& out, const chessboard_type& chessBoard);
std::ostream& operator<<(std::ostream& out, const chessboard_type& chessBoard)
{
   for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
         auto& item = chessBoard[y*8+x];
         out << (item > 0 ? "Q " : ". ");
      }
      out << "\n";
   }
   out << "\n";

   return out;
}

std::ostream& operator<<(std::ostream& out, const std::array<point, 8>& queens2d);
std::ostream& operator<<(std::ostream& out, const std::array<point, 8>& queens2d)
{
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int i = 0;
            while (i < queens2d.size()) {
                if (queens2d[i].first == x && queens2d[i].second == y) {
                    break;
                }
                ++i;
            }
            out << (i < queens2d.size() ? "Q " : ". ");
        }
        out << "\n";
    }
    out << "\n";

    return out;
}
std::ostream& operator<<(std::ostream& out, const std::array<uint8_t, 8>& arr);

class ChessBoard;
std::ostream& operator<<(std::ostream& out, const ChessBoard& brd);


class ChessBoard {
    friend std::ostream& operator<<(std::ostream& out, const ChessBoard& brd);

    static std::array<point, 8> state_to_queens2d(const chessboard_type& state) {
        std::array<point, 8> queens;

        size_t q_idx = 0;
        for (size_t i = 0; i < state.size(); ++i) {
            if (state[i]) {
                queens[q_idx] = std::make_pair<uint8_t, uint8_t>(i % 8, uint8_t(i) >> 3);
            }
        }
        return queens;
    }

    static chessboard_type queens2d_to_state(const std::array<point, 8>& queens2d) {
        chessboard_type state{};
        for (size_t i = 0; i < queens2d.size(); ++i) {
            state[queens2d[i].second * 8 + queens2d[i].first] = uint8_t(i) + 1;
        }
        return state;
    }

    static inline void queens1d_to_state(chessboard_type& state, const std::array<uint8_t, 8>& queens1d) {
        std::memset(&state[0], 0, sizeof(state[0]) * state.size());
        for (size_t i = 0; i < queens1d.size(); ++i) {
            state[queens1d[i]] = uint8_t(i) + 1;
        }
    }

    static std::array<uint8_t, 8> state_to_queens1d(const chessboard_type& state) {
        std::array<uint8_t, 8> queens1d{};
        for (size_t i = 0; i < state.size(); ++i) {
            if (state[i]) {
                queens1d[state[i] - 1] = uint8_t(i);
            }
        }
        return queens1d;
    }

    static inline uint8_t check_state(const std::array<bool, 64 * 64>& crossroads, const std::array<uint8_t, 8>& queens1d) {
        for (int i = int(queens1d.size()) - 1; i >= 0; --i) {
            for (int j = i - 1; j >= 0; --j) {
                if (crossroads[queens1d[i] * 64 + queens1d[j]]) {
                    return i + 1;
                }
            }
        }

        return 0;
    }

    static inline std::array<bool, 64 * 64> init_crossroads()
    {
        std::array<bool, 64 * 64> crossroads;

        for (int q1 = 0; q1 < 64; q1++) {
            for (int q2 = 0; q2 < 64; q2++) {
                int x1 = q1 % 8;
                int y1 = q1 / 8;
                int x2 = q2 % 8;
                int y2 = q2 / 8;
                crossroads[q1 * 64 + q2] = (x1 == x2) || (y1 == y2) || (abs(x1 - x2) == abs(y1 - y2));
            }
        }
        return crossroads;
    }

    static inline std::array<uint8_t, 9> init_child_counts() {
        std::array<uint8_t, 9> counts;

        int i = 0;
        for (; i < 8; i++) {
            counts[i] = 8 * 8 - i;
        }
        counts[i] = 0;

        return counts;
    }

    static std::vector<std::pair<point, point>> calc_queens(const std::array<uint8_t, 8>& queens1d) {
        std::vector<std::pair<point, point>> queen_pairs;
        std::array<point, 8> queens2d;

        for (int i = 0; i < queens1d.size(); ++i) {
            queens2d[i] = point(queens1d[i] % 8, queens1d[i] / 8);
        }
        queen_pairs.reserve(20);
        for (int i = 0; i < queens2d.size(); ++i) {
            for (int j = i + 1; j < queens2d.size(); ++j) {
                const auto& queen1 = queens2d[i];
                const auto& queen2 = queens2d[j];

                if (queen1.first == queen2.first ||
                    queen1.second == queen2.second ||
                    abs(queen1.first - queen2.first) == abs(queen1.second - queen2.second)) {
                    queen_pairs.emplace_back(queen1, queen2);
                }
            }
        }

        return queen_pairs;
    }
    


    /*void IDS_full(uint8_t depth_limit)
    {
        using namespace std::chrono_literals;

        // check initial state
        results.iterations++;
        results.queens1d = initial_queens1d;
        if (check_state(crossroads, results.queens1d))
        {
            results.success = true;
            return;
        }

        struct recur_state_tag {
            uint8_t depth = 0;
            std::array<uint8_t, 8> queens1d{};
        };

        std::vector<recur_state_tag> states_(9);           // stack of states
        results.memory = sizeof(recur_state_tag) * states_.size();

        uint8_t cur_depth = 0;
        depth_limit = std::min(depth_limit, uint8_t(8));
        uint8_t min_log_depth = depth_limit;

        int stack_ptr = 0;
        states_[stack_ptr].depth = 0;                           // depth = count of queens - 1
        states_[stack_ptr].queens1d = initial_queens1d;         // initial queen state

        chessboard_type cur_board{};
        queens1d_to_state(cur_board, initial_queens1d);         // initial chessboard state
        results.memory += sizeof(chessboard_type);
        //results.memory += sizeof(std::array<bool, 64 * 64>);
        //results.memory += sizeof(std::array<uint8_t, 9>);

        while (states_[stack_ptr].depth < 8) {
            // add child
            recur_state_tag& rec = states_[stack_ptr + 1];
            rec.depth = states_[stack_ptr].depth + 1;
            rec.queens1d = initial_queens1d;
            results.nodes += child_counts[rec.depth];
            stack_ptr++;
        }
        stack_ptr = depth_limit - 1;
        uint8_t cur_queen_pos = 0;
        while (true) {
            if (stack_ptr < 0) {
                results.success = false;
                return;
            }
            recur_state_tag& cur_state_ = states_[stack_ptr];
            if (check_state(crossroads, cur_state_.queens1d)) {
                results.success = true;
                results.queens1d = cur_state_.queens1d;
            }
            stack_ptr--;
            results.iterations++;

            for (int i = 0; i < cur_board.size(); i++) {
                printf("%2d", int(cur_board[i]));
            }
            printf("\n");
            std::this_thread::sleep_for(5ms);

            if (cur_state_.depth < min_log_depth) {
            }
            // check sibling (end)
            cur_queen_pos = cur_state_.queens1d[cur_state_.depth];
            cur_board[cur_queen_pos] = 0;
            // find free cell
            bool go_to_parent = false;
            do {
                cur_queen_pos++;
                if (cur_queen_pos == 64) {
                    cur_queen_pos = 0;
                }
                if (cur_queen_pos == initial_queens1d[cur_state_.depth]) {
                    cur_board[initial_queens1d[cur_state_.depth]] = cur_state_.depth + 1;
                    go_to_parent = true;
                    break;
                }
            } while (cur_board[cur_queen_pos] != 0);
            if (go_to_parent) {
                continue;
            }
            cur_state_.queens1d[cur_state_.depth] = cur_queen_pos;
            cur_board[cur_queen_pos] = cur_state_.depth + 1;
            stack_ptr++;
            results.nodes += child_counts[cur_state_.depth];
            while (states_[stack_ptr].depth < depth_limit - 1) {
                // add child
                recur_state_tag& rec = states_[stack_ptr + 1];
                rec.depth = states_[stack_ptr].depth + 1;
                int i = 0;
                while (i < rec.depth) {
                    rec.queens1d[i] = states_[stack_ptr].queens1d[i];
                    ++i;
                }
                while (i < 8) {
                    rec.queens1d[i] = initial_queens1d[i];
                    ++i;
                }
                results.nodes += child_counts[rec.depth];
                stack_ptr++;
            }
        }
    }*/


    ChessBoard() = default;
    chessboard_type initial_state{};
    std::array<uint8_t, 8> initial_queens1d{};
    std::array<bool, 64 * 64> crossroads{};
    std::array<uint8_t, 9> child_counts{};
    results_IDS_tag results{};

public:

    template<typename TContainer>
    static ChessBoard create(const TContainer& queens2d) {
       std::array<point, 8> array;
       if (is_input_valid(queens2d)) {
          std::copy(queens2d.begin(), queens2d.end(), array.begin());
       }
       return ChessBoard(array);
    }

    explicit ChessBoard(const std::array<point, 8>& queens2d) :
        initial_state(queens2d_to_state(queens2d)),
        initial_queens1d(state_to_queens1d(initial_state)) {
        crossroads = init_crossroads();
        child_counts = init_child_counts();
    }

    template <typename TContainer>
    static bool is_input_valid(const TContainer& queens) {
        return queens.size() == 8;
    }

    chessboard_type get_initial_state() {
        return initial_state;
    }

    void IDS(uint8_t depth_limit)
    {
        using namespace std::chrono_literals;

        //results.iterations++;
        results.queens1d = initial_queens1d;

        uint8_t fault_level = 0;
        if (fault_level = check_state(crossroads, results.queens1d); fault_level == 0) {
            results.success = true;
            return;
        }

        struct recur_state_tag {
            uint8_t depth = 0;
            std::array<uint8_t, 8> queens1d{};
        };

        std::vector<recur_state_tag> states_(9);
        results.memory = sizeof(recur_state_tag) * states_.size();

        uint8_t cur_depth = 0;
        depth_limit = std::min(depth_limit, uint8_t(8));
        uint8_t min_log_depth = depth_limit;

        int stack_ptr = 0;
        states_[stack_ptr].depth = 0;                           
        states_[stack_ptr].queens1d = initial_queens1d;         

        chessboard_type cur_board{};
        queens1d_to_state(cur_board, initial_queens1d);         
        results.memory += sizeof(chessboard_type);
        //results.memory += sizeof(std::array<bool, 64 * 64>);
        //results.memory += sizeof(std::array<uint8_t, 9>);

        while (states_[stack_ptr].depth < 8) {
            // add child
            recur_state_tag& rec = states_[stack_ptr + 1];
            rec.depth = states_[stack_ptr].depth + 1;
            rec.queens1d = initial_queens1d;
            results.nodes += child_counts[rec.depth];
            stack_ptr++;
        }
        stack_ptr = depth_limit - 1;
        uint8_t cur_queen_pos = 0;
        while (true) {
            if (stack_ptr < 0) {
                results.success = false;
                return;
            }
            if (fault_level = check_state(crossroads, states_[stack_ptr].queens1d); fault_level == 0) {
                results.success = true;
                results.queens1d = states_[stack_ptr].queens1d;
                return;
            }

            fault_level--;
            results.iterations++;
            if (stack_ptr > fault_level) {
                cur_board[states_[fault_level].queens1d[states_[fault_level].depth]] = -1;
                while (stack_ptr > fault_level) {
                    cur_board[states_[stack_ptr].queens1d[states_[stack_ptr].depth]] = 0;
                    cur_board[initial_queens1d[states_[stack_ptr].depth]] = states_[stack_ptr].depth + 1;
                    stack_ptr--;
                }
            }
            recur_state_tag& cur_state_ = states_[stack_ptr];
            stack_ptr--;

            static auto start = std::chrono::high_resolution_clock::now();
            const auto end = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<float> duration = end - start;
            if (duration.count() > 2)
            {
                std::cout << cur_board;
                start = end;
            }

            // check sibling (end)
            cur_queen_pos = cur_state_.queens1d[cur_state_.depth];
            cur_board[cur_queen_pos] = 0;
            // find free cell
            bool go_to_parent = false;
            do {
                cur_queen_pos++;
                if (cur_queen_pos == 64) {
                    cur_queen_pos = 0;
                }
                if (cur_queen_pos == initial_queens1d[cur_state_.depth]) {
                    cur_board[initial_queens1d[cur_state_.depth]] = cur_state_.depth + 1;
                    go_to_parent = true;
                    break;
                }
            } while (cur_board[cur_queen_pos] != 0);
            if (go_to_parent) {
                continue;
            }
            cur_state_.queens1d[cur_state_.depth] = cur_queen_pos;
            cur_board[cur_queen_pos] = cur_state_.depth + 1;
            stack_ptr++;
            results.nodes += child_counts[cur_state_.depth];
            while (states_[stack_ptr].depth < depth_limit - 1) {
                // add child
                recur_state_tag& rec = states_[stack_ptr + 1];
                rec.depth = states_[stack_ptr].depth + 1;
                int i = 0;
                while (i < rec.depth) {
                    rec.queens1d[i] = states_[stack_ptr].queens1d[i];
                    ++i;
                }
                while (i < 8) {
                    rec.queens1d[i] = initial_queens1d[i];
                    ++i;
                }
                cur_board[rec.queens1d[rec.depth]] = rec.depth + 1;
                results.nodes += child_counts[rec.depth];
                stack_ptr++;
            }
        }
    }
};

std::ostream& operator<<(std::ostream& out, const std::vector<std::pair<point, point>>& vec)
{
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            std::string c = " . ";
            int i = 0;
            while (i < vec.size()) {
                if (vec[i].first.first == x && vec[i].first.second == y) {
                    break;
                }
                ++i;
            }
            if (i < vec.size()) {
                c = " F ";
            } else {
                int i = 0;
                while (i < vec.size()) {
                    if (vec[i].second.first == x && vec[i].second.second == y) {
                        break;
                    }
                    ++i;
                }
                if (i < vec.size()) {
                    c = " S ";
                }
            }
            out << c;
        }
        out << "\n";
    }
    out << "\n";

    return out;
}

std::ostream& operator<<(std::ostream& out, const std::array<uint8_t, 8>& arr)
{
    for (int y = 0; y < 8; ++y) {
        out << int(arr[y]) << "  ";
    }
    out << "\n";

    return out;
}

std::ostream& operator<<(std::ostream& out, const ChessBoard& brd)
{
    out << "Initial board:\n\n";
    out << brd.initial_state;


    if (brd.results.success) {
        out << "Solution:\n\n";
        chessboard_type cur_board;
        ChessBoard::queens1d_to_state(cur_board, brd.results.queens1d);
        out << cur_board;
        out << "Number of iterations: " << (brd.results.iterations / 10000) << "\n"
            << "Number of expanded nodes count - " << (brd.results.nodes / 10000) << "\n"
            << "Number of nodes in memory - " << brd.results.memory << "\n" 
            << "Number of dead ends - " << brd.results.dead_ends << "\n";
    } else {
        out << "IDS was not succeeded\n";
    }
    return out;
}

using point = std::pair<uint8_t, uint8_t>;
using state_type = uint32_t;

std::ostream& operator<<(std::ostream& out, const std::vector<point>& vec);
std::ostream& operator<<(std::ostream& out, const std::array<uint8_t, 8>& arr);
std::ostream& operator<<(std::ostream& out, const std::vector<std::pair<point, point>>& vec);

std::optional<std::vector<point>> loadBoardFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << "\n";
        return std::nullopt;
    }

    std::vector<point> board;
    std::string line;
    while (std::getline(file, line)) {
        size_t pos1 = line.find('(');
        size_t pos2 = line.find(',');
        size_t pos3 = line.find(')');
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
            std::cerr << "Incorrect format in file.\n";
            return std::nullopt;
        }

        uint8_t x = std::stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
        uint8_t y = std::stoi(line.substr(pos2 + 1, pos3 - pos2 - 1));
        board.emplace_back(x, y);
    }

    if (board.size() != 8) {
        std::cerr << "File should contain 8 queens.\n";
        return std::nullopt;
    }

    return board;
}

class Board {
    friend std::ostream& operator<<(std::ostream& out, const Board& brd);
public:
    static constexpr int N = 8;
    std::array<uint8_t, N> queens;
    static uint32_t array2state(std::array<uint8_t, 8> queens)
    {
        uint32_t state = (queens[0] << 28) | (queens[1] << 24) | (queens[2] << 20) | (queens[3] << 16) | (queens[4] << 12) |
            (queens[5] << 8) | (queens[6] << 4) | (queens[7]);
        return state;
    }

    static std::array<uint8_t, 8> state2array(uint32_t state)
    {
        std::array<uint8_t, 8> queens;

        queens[0] = (state >> 28) & 0x0F;
        queens[1] = (state >> 24) & 0x0F;
        queens[2] = (state >> 20) & 0x0F;
        queens[3] = (state >> 16) & 0x0F;
        queens[4] = (state >> 12) & 0x0F;
        queens[5] = (state >> 8) & 0x0F;
        queens[6] = (state >> 4) & 0x0F;
        queens[7] = (state >> 0) & 0x0F;

        return queens;
    }

    static std::optional<std::array<uint8_t, 8>> normalize(std::vector<point> vec) {
        if (vec.size() != 8) {
            return std::nullopt;
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < i; ++j) {
                if (vec[i].first == vec[j].first) {
                    for (int n = 0; n < 8; ++n) {
                        if (n == i) {
                            continue;
                        }
                        int k = 0;
                        while (k < 8) {
                            if (vec[k].first == n) {
                                break;
                            }
                            ++k;
                        }
                        if (k == 8) {
                            vec[i].first = n;
                            break;
                        }
                    }
                }
            }
        }
        std::array<uint8_t, 8> queens;
        for (int i = 0; i < 8; ++i) {
            queens[i] = vec[i].second;
        }
        return std::optional(queens);
    }

    static std::vector<std::pair<point, point>> calculateConflicts(const std::array<uint8_t, 8>& queens) {
        std::vector<std::pair<point, point>> conflicts;

        for (int i = 0; i < queens.size(); ++i) {
            for (int j = i + 1; j < queens.size(); ++j) {
                uint8_t queen1 = queens[i];
                uint8_t queen2 = queens[j];

                if (queen1 == queen2 ||
                    abs(i - j) == abs(queen1 - queen2)) {
                    conflicts.push_back(std::make_pair(std::make_pair(i, queen1), std::make_pair(j, queen2)));
                }
            }
        }

        return conflicts;
    }

    std::vector<std::pair<Board, int>> generateSuccessors() const {
        std::vector<std::pair<Board, int>> successors;
        for (int row = 0; row < N; ++row) {
            for (int col = 0; col < N; ++col) {
                if (queens[row] == col) continue;
                Board successor = *this;
                successor.queens[row] = col;
                int h = successor.calculateConflicts(successor.queens).size();
                successors.emplace_back(successor, h);
            }
        }
        return successors;
    }

    Board() = default;

    std::vector<point> generateConflictedBoard() {
        std::set<point> uniqueQueens;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 7);

        while (uniqueQueens.size() < 8) {
            uint8_t row = static_cast<uint8_t>(dist(gen));
            uint8_t col = static_cast<uint8_t>(dist(gen));
            uniqueQueens.emplace(row, col);
        }
        return std::vector<point>(uniqueQueens.begin(), uniqueQueens.end());
    }

    Board(const std::vector<point>& q) {
        std::optional<std::array<uint8_t, 8>> queens_opt = Board::normalize(q);
        if (!queens_opt.has_value()) {
            std::cout << "error\n";
            throw std::exception();
        }
        queens = queens_opt.value();
    }

    Board(const std::array<uint8_t, 8>& q) : queens(q) {}
    Board(std::array<uint8_t, 8>&& q) : queens(std::move(q)) {}
    uint32_t GetState() const {
        return array2state(queens);
    }

    std::optional<Board> RBFS(int f_limit) {
        struct Node {
            Board board;
            int f;
            int g;
            int parent_f_limit;
        };

        auto calculate_f = [](const Board& b, int g) -> int {
            return g + static_cast<int>(b.calculateConflicts(b.queens).size());
        };

        int numberOfIterations = 0;
        int numberOfDeadEnds = 0;
        int totalExpandedNodes = 0;
        int maxNodesInMemory = 0;

        std::unordered_set<state_type> visited;
        std::stack<Node> stack;
        stack.push({ *this, calculate_f(*this, 0), 0, f_limit });

        while (!stack.empty()) {
            Node current = stack.top();
            stack.pop();
            numberOfIterations++;

            if (visited.find(current.board.GetState()) != visited.end()) {
                continue;
            }
            visited.insert(current.board.GetState());
            std::cout << "Iteration: " << numberOfIterations << "\n";
            current.board.print();

            if (current.f == current.g) {
                std::cout << "Solution found in " << numberOfIterations << " iterations.\n";
                std::cout << "Number of dead ends: " << numberOfDeadEnds << "\n";
                std::cout << "Number of expanded nodes: " << totalExpandedNodes << "\n";
                std::cout << "Number of nodes in memory: " << maxNodesInMemory << "\n";
                return current.board;
            }

            auto successors = current.board.generateSuccessors();
            std::vector<Node> childNodes;
            bool hasValidSuccessor = false;

            for (const auto& [childBoard, h_value] : successors) {
                int g = current.g + 1;
                int f = g + h_value;
                if (f <= current.parent_f_limit) {
                    childNodes.push_back({ childBoard, f, g, current.parent_f_limit });
                    hasValidSuccessor = true;
                }
            }

            if (!hasValidSuccessor) {
                numberOfDeadEnds++;
            } else {
                std::sort(childNodes.begin(), childNodes.end(),
                    [](const Node& a, const Node& b) { return a.f < b.f; });

                for (size_t i = 0; i < childNodes.size(); ++i) {
                    Node& child = childNodes[i];
                    int siblingF = (i + 1 < childNodes.size()) ? childNodes[i + 1].f : INT_MAX;
                    int newLimit = std::min(current.parent_f_limit, siblingF);

                    if (child.f > current.parent_f_limit) {
                        numberOfDeadEnds++;
                        continue;
                    }
                    child.parent_f_limit = newLimit;

                    if (stack.size() >= 64) {
                        break;
                    }
                    stack.push(child);
                    totalExpandedNodes++;
                }
            }
            maxNodesInMemory = std::max(maxNodesInMemory, static_cast<int>(stack.size()));
        }

        std::cout << "Solution not found after " << numberOfIterations << " iterations.\n";
        return std::nullopt;
    }

    void print() const {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                std::cout << (queens[i] == j ? "Q " : ". ");
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
};

std::ostream& operator<<(std::ostream& out, const std::vector<point>& vec)
{
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int i = 0;
            while (i < vec.size()) {
                if (vec[i].first == x && vec[i].second == y) {
                    break;
                }
                ++i;
            }
            out << (i < vec.size() ? "Q " : ". ");
        }
        out << "\n";
    }
    out << "\n";

    return out;
}

int main() {
    std::cout << "Choose method of placement of queens:\n";
    std::cout << "1. Load from file\n";
    std::cout << "2. Generate randomly\n";
    int choice;
    std::cin >> choice;

    std::vector<point> brd;
    if (choice == 1) {
        std::string filename;
        std::cout << "Enter file name: ";
        std::cin >> filename;
        auto loadedBoard = loadBoardFromFile(filename);
        if (!loadedBoard) {
            std::cerr << "Didn't manage to load the placement of queens.\n";
            return 1;
        }
        brd = loadedBoard.value();
    } else if (choice == 2) {
        Board tempBoard;
        brd = tempBoard.generateConflictedBoard();
    } else {
        std::cerr << "Error: Incorrect choice.\n";
        return 1;
    }
    std::cout << "Initial state:\n" << brd;
    Board board(brd);
    std::cout << "Choose search method:\n";
    std::cout << "1. Iterative Deepening Search (IDS)\n";
    std::cout << "2. Recursive Best-First Search (RBFS)\n";
    int searchChoice;
    std::cin >> searchChoice;
    if (searchChoice == 1) {

        if (!ChessBoard::is_input_valid(brd)) {
            std::cout << "Input data is incorrect\n";
            return -1;
        }
        ChessBoard board = ChessBoard::create(brd);

        std::cout << "Initial board:\n";
        std::cout << board.get_initial_state();

        auto start = std::chrono::high_resolution_clock::now();
        uint32_t depth_limit = 8;
        board.IDS(depth_limit);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;
        std::cout << "IDS time - " << duration.count() << " seconds.\n";

        std::cout << "Chessboard:\n";
        std::cout << board;
    } else if (searchChoice == 2) {

        int f_limit = INT_MAX;
        auto solution = board.RBFS(f_limit);
    } else {
        std::cerr << "Error: Incorrect choice of search method.\n";
        return 1;
    }
    return 0;
}