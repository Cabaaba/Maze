#include<stdio.h>
#include<vector>
#include<string>
#include<map>
#include<queue>
#include<algorithm>
#include<stdlib.h>
#include <windows.h>
#include<conio.h>
#include<set>
using namespace std;

enum Cell
{
	Wall,
	Empty,
	Start,
	Finish
};
enum Direction
{
	Left,
	Right,
	Up,
	Down,
	Last // because it's C++
};
struct Point
{
	int x,y;
	Point(int x=0, int y=0)
	{
		this->x=x;
		this->y=y;
	}
};
//--------------------------------------------------------------------------------------------------------------------------------
//To use Point as key of std::map. I'm too lazy to write a comparison class
//--------------------------------------------------------------------------------------------------------------------------------
static bool operator <(Point f, Point s)
{
	if(f.x!=s.x)
	{
		return f.x<s.x;
	}
	return f.y<s.y;
}
static bool operator >(Point f, Point s)
{
	if(f.x!=s.x)
	{
		return f.x>s.x;
	}
	return f.y>s.y;
}
//--------------------------------------------------------------------------------------------------------------------------------
static Point operator +(Point f, Point s)
{
	return Point(f.x+s.x,f.y+s.y);
}
static Point operator -(Point f, Point s)
{
	return Point(f.x-s.x,f.y-s.y);
}


//--------------------------------------------------------------------------------------------------------------------------------
//For testing
//--------------------------------------------------------------------------------------------------------------------------------
vector<string> maze;

void fillMaze()
{
	char c;
	string row="";
	while(scanf("%c",&c)!=EOF)
	{
		if(c=='\n')
		{
			maze.push_back(row);
			row.clear();
		}
		else
		{
			row+=c;
		}
	}
}
Cell getCell(Point position)
{
	//Should be replaced with hardware handling
	if(position.y<0||position.y>=maze.size())
	{
		return Empty;
	}
	if(position.x<0||position.x>=maze[position.y].length())
	{
		return Empty;
	}
	char cellSymbol = maze[position.y][position.x];
	if(cellSymbol=='#')
	{
		return Wall;
	}
	else if(cellSymbol=='f')
	{
		return Finish;
	}
	else if(cellSymbol=='s')
	{
		return Start;
	}
	else if(cellSymbol==' ')
	{
		return Empty;
	}
	throw "Unexpected maze symbol";
}
Point getStartPoint()
{
	for(int i=0;i<maze.size();++i)
	{
		for(int j=0;j<maze[i].length();++j)
		{
			if(maze[i][j]=='s')
			{
				return Point(j,i);
			}
		}
	}
	throw "Start point was not found";
}
//--------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------
//NEVER write code this way
//Direction probably should be class or several points with unique name.
//Enum was a bad idea.
//--------------------------------------------------------------------------------------------------------------------------------
map<Direction, Point> shiftByDirection =
{
	{Left, Point(-1,0)},
	{Right, Point(1,0)},
	{Up, Point(0,-1)},
	{Down, Point(0,1)}
};
map<Point, Direction> directionByShift =
{
	{Point(-1,0), Left},
	{Point(1,0), Right},
	{Point(0,-1), Up},
	{Point(0,1), Down}
};
//--------------------------------------------------------------------------------------------------------------------------------
class Vertex
{
public:
	//Position of vertex in a 2D grid.
	Point pos;
	//What is inside
	Cell cell;
	//Whether a robot walked through the vertex or not
	bool isVisited;
	//used internally by bfs aka getNextVertices. Normally it would optimize code, 
	//but given other implementation details, like using set for visited vertices list, this can be replaced by map<Vertex*,Vertex*> inside getNextVertices
	Vertex* previous;
	Vertex(Point pos, Cell cell, bool isVisited, Vertex* previous=NULL)
	{
		this->pos=pos;
		this->cell=cell;
		this->isVisited=isVisited;
		this->previous=previous;
	}
};
//Processed vertices by their positions
map<Point, Vertex*> graph;
//If a vertex with position "pos" was already processed, return it. Otherwise, return a new vertex
Vertex* getOrCreateVertex(Point pos)
{
	if(graph.count(pos)==0)
	{
		graph[pos]=new Vertex(pos, getCell(pos), false);
	}
	return graph[pos];
}
//Vertices adjacent to "vert"
vector<Vertex*> getNeighbours(Vertex* vert)
{
	vector<Vertex*> res;
	for(int dir = Left; dir!=Last; ++dir)
	{
		Point neighbourPos=vert->pos+shiftByDirection[static_cast<Direction>(dir)];
		res.push_back(getOrCreateVertex(neighbourPos));
	}
	return res;
}
//Vertices adjacent to "vert", which can be occupied by player
vector<Vertex*> getWalkableNeighbours(Vertex* vert)
{
	vector<Vertex*> tmp=getNeighbours(vert);
	vector<Vertex*> res;
	for(vector<Vertex*>::iterator i = tmp.begin();i!=tmp.end();++i)
	{
		if((*i)->cell!=Wall)
		{
			res.push_back(*i);
		}
	}
	return res;
}
//Path from "start" to "cur"
vector<Vertex*> restorePath(Vertex* start, Vertex* cur)
{
	vector<Vertex*> res;
	while(true)
	{
		res.push_back(cur);
		if(cur==start)
		{
			break;
		}
		cur=cur->previous;
	}
	reverse(res.begin(),res.end());
	return res;
}
//Finds a list of vertices from "start" to "end"
//where "start" is a vertex with "pos" coordinate
//"end" is the closest vertex to "start" from set of vertices those are either 
//unexplored or represent finish of the maze
//https://en.wikipedia.org/wiki/Breadth-first_search
vector<Vertex*> getNextVertices(Point pos)
{
	//Vertices visited by this bfs. set can be replaced for optimization
	set<Vertex*> was;
	//queue of vertices
	queue<Vertex*> q;
	Vertex* start = getOrCreateVertex(pos);
	q.push(start);
	while(!q.empty())
	{
		Vertex* current=q.front();
		was.insert(current);
		q.pop();
		if(current->isVisited==false)
		{
			return restorePath(start, current);
		}
		vector<Vertex*> neighbours=getWalkableNeighbours(current);
		for(vector<Vertex*>::iterator i = neighbours.begin();i!=neighbours.end();++i)
		{
			if(was.count(*i)!=0)
			{
				continue;
			}
			(*i)->previous=current;
			if((*i)->cell==Finish)
			{
				return restorePath(start,(*i));
			}
			q.push(*i);
		}
	}
	return vector<Vertex*>();
}
//Output cell representation
map<Cell,char> symbolOfCell =
{
	{Wall,'#'},
	{Empty,'.'},
	{Start,'s'},
	{Finish,'f'}
};
//Draws a (2*SHIFT+1) x (2*SHIFT+1) maze region around robot
void display(Vertex* v)
{
	system("cls");
	Point pos = v->pos;
	const int SHIFT=10;
	string buffer="";
	for(int y=v->pos.y-SHIFT;y<=v->pos.y+SHIFT;++y)
	{
		
		for(int x=v->pos.x-SHIFT;x<=v->pos.x+SHIFT;++x)
		{
			if(x==pos.x&&y==pos.y)
			{
				buffer+="@";
				continue;
			}
			Point current=Point(x,y);
			if(graph.count(current)==0)
			{
				buffer+=" ";
			}
			else
			{
				buffer+=symbolOfCell[graph[current]->cell];
			}
			
		}
		buffer+="\n";
	}
	printf("%s",buffer.c_str());
	Sleep(150);
}

int main()
{
	freopen("input.txt","r",stdin);
	fillMaze();
	Point pos=getStartPoint();
	getOrCreateVertex(pos)->isVisited=true;
	vector<Vertex*> path;
	while(true)
	{
		path = getNextVertices(pos);
		for(vector<Vertex*>::iterator i = path.begin();i!=path.end();++i)
		{
			//fprintf(stderr,"position={%d; %d}\n",(*i)->pos.x,(*i)->pos.y);
			display(*i);
			pos=(*i)->pos;
			(*i)->isVisited=true;
			
			if(getCell(pos)==Finish)
			{
				goto end;
			}
		}
		if(path.empty())
		{
			goto end;
		}
	}
	end:
	printf("\nPress any key to exit\n");
	while(!kbhit());
}
