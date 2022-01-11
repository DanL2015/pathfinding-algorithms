#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
using namespace std;
using namespace sf;

// Visual importance
Vector2f windowSize = Vector2f(854, 480);
RenderWindow window(VideoMode(windowSize.x, windowSize.y), "pathfinding algorithms", Style::Resize | Style::Close);
Font pixel;
const int padding = 16;
const int fontSize = 12;

// Important variables for algorithm
const int maxWeight = 9;
Vector2f startPos;
Vector2f endPos;
vector<vector<int>> grid;
vector<vector<bool>> walls[4]; //-y, -x, +y, +x
vector<vector<bool>> visited;
int algorithmNum = 0;
vector<string> algorithms = {"breadth first search", "depth first search", "dijkstra", "a star"};
int gridTypeNum = 0;
vector<string> gridTypes = {"blank", "unweighted maze", "random weighted", "weighted maze"};
Clock globalClock;
Time movementSpeed;
bool reachedEnd = false;

struct dinfo
{
    Vector2f pos;
    int d;
    bool operator<(const dinfo &o) const
    {
        return d > o.d;
    }
};

struct ainfo
{
    Vector2f pos;
    int d;
    int f;
    bool operator<(const ainfo &o) const
    {
        return f > o.f;
    }
};

void delay(int n)
{
    n = n / 4;
    globalClock.restart();
    while (globalClock.getElapsedTime() < milliseconds(n))
    {
    }
    return;
}

void init()
{
    reachedEnd = false;
    startPos = Vector2f(-1, -1);
    endPos = Vector2f(-1, -1);
    srand(time(NULL));
    globalClock.restart();
    movementSpeed = milliseconds(20);
    pixel.loadFromFile("assets/PixelFJVerdana.ttf");
    grid.assign(windowSize.y / 16, vector<int>());
    for (int i = 0; i < windowSize.y / 16; i++)
    {
        grid[i].assign(windowSize.x / 16, 0);
    }
    visited.assign(windowSize.y / 16, vector<bool>());
    for (int i = 0; i < windowSize.y / 16; i++)
    {
        for (int j = 0; j < windowSize.x / 16; j++)
        {
            visited[i].push_back(false);
        }
    }
    for (int k = 0; k < 4; k++)
    {
        walls[k].assign(windowSize.y / 16, vector<bool>());
        for (int i = 0; i < windowSize.y / 16; i++)
        {
            for (int j = 0; j < windowSize.x / 16; j++)
            {
                walls[k][i].push_back(false);
            }
        }
    }
}

void initializeText(Text &text)
{
    text.setFont(pixel);
    text.setCharacterSize(fontSize);
}

void drawGrid()
{
    Event event;
    while (window.pollEvent(event))
    {
        if (event.type == Event::Closed)
        {
            window.close();
        }
    }
    if (startPos.x != -1 && startPos.y != -1)
    {
        RectangleShape temp;
        temp.setFillColor(Color::Blue);
        temp.setSize(Vector2f(16, 16));
        temp.setPosition(Vector2f(startPos.x * 16, startPos.y * 16));
        window.draw(temp);
    }
    if (endPos.x != -1 && endPos.y != -1)
    {
        RectangleShape temp;
        if (reachedEnd)
        {
            temp.setFillColor(Color::Green);
        }
        else
        {
            temp.setFillColor(Color::Red);
        }
        temp.setSize(Vector2f(16, 16));
        temp.setPosition(Vector2f(endPos.x * 16, endPos.y * 16));
        window.draw(temp);
    }
    for (int i = 0; i < grid.size(); i++)
    {
        for (int j = 0; j < grid[i].size(); j++)
        {
            RectangleShape tempVertical;
            RectangleShape tempHorizontal;
            tempVertical.setSize(Vector2f(2, 16));
            tempVertical.setOrigin(Vector2f(1, 0));
            tempVertical.setFillColor(Color::White);
            tempHorizontal.setSize(Vector2f(16, 2));
            tempHorizontal.setOrigin(Vector2f(0, 1));
            tempHorizontal.setFillColor(Color::White);
            if (!walls[0][i][j])
            {
                // wall in -y
                tempVertical.setPosition(Vector2f(j * 16, i * 16));
                window.draw(tempVertical);
            }
            if (!walls[1][i][j])
            {
                // wall in -x
                tempHorizontal.setPosition(Vector2f(j * 16, i * 16));
                window.draw(tempHorizontal);
            }
            if (!walls[2][i][j])
            {
                // wall in +y
                tempVertical.setPosition(Vector2f((j + 1) * 16, i * 16));
                window.draw(tempVertical);
            }
            if (!walls[3][i][j])
            {
                // wall in +x
                tempHorizontal.setPosition(Vector2f(j * 16, (i + 1) * 16));
                window.draw(tempHorizontal);
            }
        }
    }
    for (int i = 0; i < grid.size(); i++)
    {
        for (int j = 0; j < grid[i].size(); j++)
        {
            if (grid[i][j] != 0)
            {
                Text tempText;
                initializeText(tempText);
                tempText.setFillColor(Color::White);
                tempText.setCharacterSize(8);
                tempText.setString(to_string(grid[i][j]));
                tempText.setOrigin(Vector2f(tempText.getLocalBounds().width / 2, tempText.getLocalBounds().height / 2 - 4));
                tempText.setPosition(Vector2f(j * 16 + 8, i * 16 + 8));
                window.draw(tempText);
            }
        }
    }
}

void generateMap()
{
    switch (gridTypeNum)
    {
    case (0):
    {
        // blank (delete all walls)
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < windowSize.y / 16; j++)
            {
                for (int k = 0; k < windowSize.x / 16; k++)
                {
                    walls[i][j][k] = true;
                }
            }
        }
        return;
        break;
    }
    case (1):
    {
        // maze generation using random dfs starting from 0, 0
        int dy[4] = {-1, 0, 1, 0};
        int dx[4] = {0, -1, 0, 1};
        stack<pair<int, int>> s;
        s.push({0, 0});
        visited[0][0] = true;
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        while (!s.empty())
        {
            pair<int, int> curNode = s.top();
            window.clear();
            drawGrid();
            window.display();
            s.pop();
            // randomize the next node;
            vector<int> temp = {0, 1, 2, 3};
            vector<int> n;
            while (temp.size() > 0)
            {
                int ni = rand() % temp.size();
                n.push_back(temp[ni]);
                temp.erase(temp.begin() + ni);
            }
            for (int i = 0; i < n.size(); i++)
            {
                int ny = curNode.first + dy[n[i]];
                int nx = curNode.second + dx[n[i]];
                if (ny < 0 || ny >= windowSize.y / 16 || nx < 0 || nx >= windowSize.x / 16)
                {
                    continue;
                }
                if (!visited[ny][nx])
                {
                    if (n[i] == 0 || n[i] == 2)
                    {
                        walls[(n[i] + 1) % 4][curNode.first][curNode.second] = true;
                        walls[(n[i] + 3) % 4][ny][nx] = true;
                    }
                    if (n[i] == 1 || n[i] == 3)
                    {
                        walls[(n[i] + 3) % 4][curNode.first][curNode.second] = true;
                        walls[(n[i] + 1) % 4][ny][nx] = true;
                    }
                    visited[ny][nx] = true;
                    s.push({ny, nx});
                    grid[ny][nx] = 0;
                }
            }
        }
        return;
        break;
    }
    case (2):
    {
        // random weighted
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < walls[i].size(); j++)
            {
                for (int k = 0; k < walls[i][j].size(); k++)
                {
                    walls[i][j][k] = true;
                }
            }
        }
        for (int i = 0; i < grid.size(); i++)
        {
            for (int j = 0; j < grid[i].size(); j++)
            {
                window.clear();
                grid[i][j] = (rand() % maxWeight) + 1;
                drawGrid();
                window.display();
            }
        }
        return;
        break;
    }
    case (3):
    {
        int dy[4] = {-1, 0, 1, 0};
        int dx[4] = {0, -1, 0, 1};
        stack<pair<int, int>> s;
        s.push({0, 0});
        visited[0][0] = true;
        grid[0][0] = (rand() % maxWeight) + 1;
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        while (!s.empty())
        {
            pair<int, int> curNode = s.top();
            window.clear();
            drawGrid();
            window.display();
            s.pop();
            vector<int> temp = {0, 1, 2, 3};
            vector<int> n;
            while (temp.size() > 0)
            {
                int ni = rand() % temp.size();
                n.push_back(temp[ni]);
                temp.erase(temp.begin() + ni);
            }
            for (int i = 0; i < n.size(); i++)
            {
                int ny = curNode.first + dy[n[i]];
                int nx = curNode.second + dx[n[i]];
                if (ny < 0 || ny >= windowSize.y / 16 || nx < 0 || nx >= windowSize.x / 16)
                {
                    continue;
                }
                if (!visited[ny][nx])
                {
                    if (n[i] == 0 || n[i] == 2)
                    {
                        walls[(n[i] + 1) % 4][curNode.first][curNode.second] = true;
                        walls[(n[i] + 3) % 4][ny][nx] = true;
                    }
                    if (n[i] == 1 || n[i] == 3)
                    {
                        walls[(n[i] + 3) % 4][curNode.first][curNode.second] = true;
                        walls[(n[i] + 1) % 4][ny][nx] = true;
                    }
                    visited[ny][nx] = true;
                    s.push({ny, nx});
                    grid[ny][nx] = (rand() % maxWeight) + 1;
                }
            }
        }
        return;
        break;
    }
    }
    return;
}

int welcomeScreen()
{
    Text welcomeText;
    initializeText(welcomeText);
    welcomeText.setString("path finding visualizer");
    welcomeText.setPosition(Vector2f(padding, welcomeText.getLocalBounds().height));

    Text algorithmNumText;
    initializeText(algorithmNumText);
    algorithmNumText.setString("algorithm: " + algorithms[algorithmNum]);
    algorithmNumText.setPosition(Vector2f(padding, algorithmNumText.getLocalBounds().height + padding * 2));

    Text gridTypeNumText;
    initializeText(gridTypeNumText);
    gridTypeNumText.setString("grid type: " + gridTypes[gridTypeNum]);
    gridTypeNumText.setPosition(Vector2f(padding, gridTypeNumText.getLocalBounds().height + padding * 4));

    Text instructionText;
    initializeText(instructionText);
    instructionText.setString("choose the algorithm and grid type, then press enter.\nnote that dfs and bfs don't consider weights.");
    instructionText.setPosition(Vector2f(padding, instructionText.getLocalBounds().height + padding * 6));

    int choices = 2;
    int curChoice = 0;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case (Event::Closed):
            {
                window.close();
                break;
            }
            case (Event::KeyPressed):
            {
                switch (event.key.code)
                {
                case (Keyboard::Enter):
                {
                    generateMap();
                    return 1;
                    break;
                }
                case (Keyboard::W):
                {
                    curChoice++;
                    curChoice %= choices;
                    break;
                }
                case (Keyboard::Up):
                {
                    curChoice++;
                    curChoice %= choices;
                    break;
                }
                case (Keyboard::A):
                {
                    if (curChoice == 0)
                    {
                        algorithmNum--;
                        algorithmNum = (algorithmNum < 0) ? algorithms.size() - 1 : algorithmNum;
                    }
                    else if (curChoice == 1)
                    {
                        gridTypeNum--;
                        gridTypeNum = (gridTypeNum < 0) ? gridTypes.size() - 1 : gridTypeNum;
                    }
                    break;
                }
                case (Keyboard::Left):
                {
                    if (curChoice == 0)
                    {
                        algorithmNum--;
                        algorithmNum = (algorithmNum < 0) ? algorithms.size() - 1 : algorithmNum;
                    }
                    else if (curChoice == 1)
                    {
                        gridTypeNum--;
                        gridTypeNum = (gridTypeNum < 0) ? gridTypes.size() - 1 : gridTypeNum;
                    }
                    break;
                }
                case (Keyboard::S):
                {
                    curChoice--;
                    curChoice = (curChoice < 0) ? choices - 1 : curChoice;
                    break;
                }
                case (Keyboard::Down):
                {
                    curChoice--;
                    curChoice = (curChoice < 0) ? choices - 1 : curChoice;
                    break;
                }
                case (Keyboard::D):
                {
                    if (curChoice == 0)
                    {
                        algorithmNum++;
                        algorithmNum %= algorithms.size();
                    }
                    else if (curChoice == 1)
                    {
                        gridTypeNum++;
                        gridTypeNum %= gridTypes.size();
                    }
                    break;
                }
                case (Keyboard::Right):
                {
                    if (curChoice == 0)
                    {
                        algorithmNum++;
                        algorithmNum %= algorithms.size();
                    }
                    else if (curChoice == 1)
                    {
                        gridTypeNum++;
                        gridTypeNum %= gridTypes.size();
                    }
                    break;
                }
                }
                break;
            }
            }
        }

        switch (curChoice)
        {
        case (0):
        {
            algorithmNumText.setFillColor(Color::Green);
            gridTypeNumText.setFillColor(Color::White);
            break;
        }
        case (1):
        {
            algorithmNumText.setFillColor(Color::White);
            gridTypeNumText.setFillColor(Color::Green);
            break;
        }
        }

        algorithmNumText.setString("algorithm: " + algorithms[algorithmNum]);
        gridTypeNumText.setString("grid type: " + gridTypes[gridTypeNum]);
        window.clear();
        window.draw(welcomeText);
        window.draw(algorithmNumText);
        window.draw(gridTypeNumText);
        window.draw(instructionText);
        window.display();
    }

    return 0;
}

void choosePosition()
{
    Text instructionText;
    initializeText(instructionText);
    instructionText.setString("click to set the starting node.");
    instructionText.setFillColor(Color::Black);
    instructionText.setCharacterSize(fontSize / 2);
    instructionText.setPosition(Vector2f(padding / 2, windowSize.y - instructionText.getLocalBounds().height));

    RectangleShape instructionTextBackground;
    instructionTextBackground.setSize(Vector2f(instructionText.getLocalBounds().width + padding, instructionText.getLocalBounds().height + padding / 2));
    instructionTextBackground.setFillColor(Color::White);
    instructionTextBackground.setPosition(Vector2f(0, windowSize.y - instructionTextBackground.getLocalBounds().height));

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == Mouse::Left)
                {
                    if (startPos.x == -1 && startPos.y == -1)
                    {
                        startPos.x = Mouse::getPosition(window).x / 16;
                        startPos.y = Mouse::getPosition(window).y / 16;
                        instructionText.setString("click to set the ending node.");
                        instructionTextBackground.setSize(Vector2f(instructionText.getLocalBounds().width + padding, instructionText.getLocalBounds().height + padding / 2));
                    }
                    else
                    {
                        endPos.x = Mouse::getPosition(window).x / 16;
                        endPos.y = Mouse::getPosition(window).y / 16;
                    }
                }
            }
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape)
                {
                    startPos = Vector2f(-1, -1);
                    endPos = Vector2f(-1, -1);
                    instructionText.setString("click to set the starting node.");
                    instructionTextBackground.setSize(Vector2f(instructionText.getLocalBounds().width + padding, instructionText.getLocalBounds().height + padding / 2));
                }
                if (event.key.code == Keyboard::Enter)
                {
                    if (endPos.x != -1 && endPos.y != -1)
                    {
                        return;
                    }
                }
            }
        }
        window.clear();
        drawGrid();
        window.draw(instructionTextBackground);
        window.draw(instructionText);
        window.display();
    }
}

void dfs()
{
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            visited[i][j] = false;
        }
    }
    int dy[4] = {-1, 0, 1, 0};
    int dx[4] = {0, -1, 0, 1};
    stack<Vector2f> s;
    s.push(startPos);
    visited[startPos.y][startPos.x] = true;
    while (!s.empty())
    {
        int cx = s.top().x;
        int cy = s.top().y;
        window.clear();
        for (int i = 0; i < visited.size(); i++)
        {
            for (int j = 0; j < visited[i].size(); j++)
            {
                if (visited[i][j])
                {
                    RectangleShape temp;
                    temp.setFillColor(Color::Cyan);
                    temp.setSize(Vector2f(16, 16));
                    temp.setPosition(Vector2f(j * 16, i * 16));
                    window.draw(temp);
                }
            }
        }
        drawGrid();
        window.display();
        delay(2);
        if (cx == endPos.x && cy == endPos.y)
        {
            reachedEnd = true;
            return;
        }
        s.pop();
        for (int i = 0; i < 4; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= windowSize.x / 16 || ny < 0 || ny >= windowSize.y / 16)
            {
                continue;
            }
            // check for walls
            if ((i == 0 || i == 2) && !walls[(i + 1) % 4][cy][cx])
            {
                continue;
            }
            if ((i == 1 || i == 3) && !walls[(i + 3) % 4][cy][cx])
            {
                continue;
            }
            if (!visited[ny][nx])
            {
                visited[ny][nx] = true;
                s.push(Vector2f(nx, ny));
            }
        }
    }
}

void bfs()
{
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            visited[i][j] = false;
        }
    }
    int dy[4] = {-1, 0, 1, 0};
    int dx[4] = {0, -1, 0, 1};
    queue<Vector2f> q;
    q.push(startPos);
    visited[startPos.y][startPos.x] = true;
    while (!q.empty())
    {
        int cx = q.front().x;
        int cy = q.front().y;
        window.clear();
        for (int i = 0; i < visited.size(); i++)
        {
            for (int j = 0; j < visited[i].size(); j++)
            {
                if (visited[i][j])
                {
                    RectangleShape temp;
                    temp.setFillColor(Color::Cyan);
                    temp.setSize(Vector2f(16, 16));
                    temp.setPosition(Vector2f(j * 16, i * 16));
                    window.draw(temp);
                }
            }
        }
        drawGrid();
        window.display();
        delay(2);
        if (cx == endPos.x && cy == endPos.y)
        {
            reachedEnd = true;
            return;
        }
        q.pop();
        for (int i = 0; i < 4; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= windowSize.x / 16 || ny < 0 || ny >= windowSize.y / 16)
            {
                continue;
            }
            // check for walls
            if ((i == 0 || i == 2) && !walls[(i + 1) % 4][cy][cx])
            {
                continue;
            }
            if ((i == 1 || i == 3) && !walls[(i + 3) % 4][cy][cx])
            {
                continue;
            }
            if (!visited[ny][nx])
            {
                visited[ny][nx] = true;
                q.push(Vector2f(nx, ny));
            }
        }
    }
}

void dijkstra()
{
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            visited[i][j] = false;
        }
    }
    int dy[4] = {-1, 0, 1, 0};
    int dx[4] = {0, -1, 0, 1};
    priority_queue<dinfo> q;
    q.push(dinfo{Vector2f(startPos.x, startPos.y), 0});
    visited[startPos.y][startPos.x] = true;
    while (!q.empty())
    {
        int cx = q.top().pos.x;
        int cy = q.top().pos.y;
        int cd = q.top().d;
        window.clear();
        for (int i = 0; i < visited.size(); i++)
        {
            for (int j = 0; j < visited[i].size(); j++)
            {
                if (visited[i][j])
                {
                    RectangleShape temp;
                    temp.setFillColor(Color::Cyan);
                    temp.setSize(Vector2f(16, 16));
                    temp.setPosition(Vector2f(j * 16, i * 16));
                    window.draw(temp);
                }
            }
        }
        drawGrid();
        window.display();
        delay(2);
        if (cx == endPos.x && cy == endPos.y)
        {
            reachedEnd = true;
            cout << "Shortest path was: " << cd << endl;
            return;
        }
        q.pop();
        for (int i = 0; i < 4; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= windowSize.x / 16 || ny < 0 || ny >= windowSize.y / 16)
            {
                continue;
            }
            // check for walls
            if ((i == 0 || i == 2) && !walls[(i + 1) % 4][cy][cx])
            {
                continue;
            }
            if ((i == 1 || i == 3) && !walls[(i + 3) % 4][cy][cx])
            {
                continue;
            }
            if (!visited[ny][nx])
            {
                int nd = cd + grid[ny][nx];
                visited[ny][nx] = true;
                q.push(dinfo{Vector2f(nx, ny), nd});
            }
        }
    }
}

int h(Vector2f pos)
{
    return (pos.x - endPos.x) * (pos.x - endPos.x) + (pos.y - endPos.y) * (pos.y - endPos.y);
}

void astar()
{
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            visited[i][j] = false;
        }
    }
    int dy[4] = {-1, 0, 1, 0};
    int dx[4] = {0, -1, 0, 1};
    priority_queue<ainfo> q;
    q.push(ainfo{
        Vector2f(startPos.x, startPos.y),
        0, h(Vector2f(0, 0))});
    visited[startPos.y][startPos.x] = true;
    while (!q.empty())
    {
        int cx = q.top().pos.x;
        int cy = q.top().pos.y;
        int cd = q.top().d;
        window.clear();
        for (int i = 0; i < visited.size(); i++)
        {
            for (int j = 0; j < visited[i].size(); j++)
            {
                if (visited[i][j])
                {
                    RectangleShape temp;
                    temp.setFillColor(Color::Cyan);
                    temp.setSize(Vector2f(16, 16));
                    temp.setPosition(Vector2f(j * 16, i * 16));
                    window.draw(temp);
                }
            }
        }
        drawGrid();
        window.display();
        delay(2);
        if (cx == endPos.x && cy == endPos.y)
        {
            reachedEnd = true;
            cout << "Shortest path was: " << cd << endl;
            return;
        }
        q.pop();
        for (int i = 0; i < 4; i++)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= windowSize.x / 16 || ny < 0 || ny >= windowSize.y / 16)
            {
                continue;
            }
            // check for walls
            if ((i == 0 || i == 2) && !walls[(i + 1) % 4][cy][cx])
            {
                continue;
            }
            if ((i == 1 || i == 3) && !walls[(i + 3) % 4][cy][cx])
            {
                continue;
            }
            if (!visited[ny][nx])
            {
                int nd = cd + grid[ny][nx];
                visited[ny][nx] = true;
                q.push(ainfo{Vector2f(nx, ny), nd, nd + h(Vector2f(nx, ny))});
            }
        }
    }
}

int main()
{
    init();
    int menu = welcomeScreen();
    if (!menu)
    {
        return 0;
    }
    choosePosition();
    switch (algorithmNum)
    {
    case (0):
    {
        // bfs
        bfs();
        break;
    }
    case (1):
    {
        // dfs
        dfs();

        break;
    }
    case (2):
    {
        // dijkstra
        dijkstra();
        break;
    }
    case (3):
    {
        // astar
        astar();
        break;
    }
    }
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape)
                {
                    main();
                }
            }
        }
        window.clear();
        for (int i = 0; i < visited.size(); i++)
        {
            for (int j = 0; j < visited[i].size(); j++)
            {
                if (visited[i][j])
                {
                    RectangleShape temp;
                    temp.setFillColor(Color::Cyan);
                    temp.setSize(Vector2f(16, 16));
                    temp.setPosition(Vector2f(j * 16, i * 16));
                    window.draw(temp);
                }
            }
        }
        drawGrid();
        window.display();
    }
    return 0;
}