#include <string>
#include <vector>
#include <iostream>
#include <map>

#include <assert.h>

using namespace std;

/// tris2quads.cpp
/// Convert an .obj file containing triangles (via stdin) to quads (via stdout)
/// by using the first few steps of the Catmull-Clark algorithm.
/// How to build: g++ -Wall -o tris2quads tris2quads.cpp

class Vertex {
public:
	Vertex() {
        id = 0;
	}
	
	Vertex &operator*=(double d) {
		x *= d; y *= d; z *= d;
		return *this;
	}
	int operator==(const Vertex &V) {
		return ((x == V.x) &&
				(y == V.y) &&
				(z == V.z)); }
	friend Vertex operator-(Vertex v1, Vertex v2);
	friend Vertex operator+(Vertex v1, Vertex v2);
	friend ostream &operator<<(ostream &os, Vertex &v);
	friend istream &operator>>(istream &is, Vertex &v);
	
//private:
	double x, y, z;
	unsigned int id;
	static unsigned int vertex_id;
};

unsigned int Vertex::vertex_id = 0;

class Face {
public:
	Face() {
	}
	void quadize();
	Vertex *centroid();
	
	friend ostream &operator<<(ostream &os, Face &f);
	friend istream &operator>>(istream &is, Face &f);
	
private:
	unsigned v1, v2, v3;
	/// Capital letters are pointers to the vertices!
	Vertex *V1, *V2, *V3;
};

class Quad {
public:
	Vertex *W, *X, *Y, *Z;
	friend ostream &operator<<(ostream &os, Quad &q);
};

vector<Vertex>                     vlist;
vector<Face>                       flist;
vector<Quad>                       qlist;
map<pair<Vertex*,Vertex*>,Vertex*> addedVertices;


int main()
{
	/// Push a "fake" vertex in so we start at index 1
	vlist.push_back(Vertex());
	
	while (1) {
		char c;
		
		if (!cin.good())
			break;
		
		c = cin.peek();
		
		if (c == 'v') {
			/// Add the vertex to our vertex list
			Vertex v;
			cin >> v;
			vlist.push_back(v);
			cout << vlist.back() << endl;
		}
		
		else if (c == 'f') {
			/// Add the face to our face list
			Face f;
			cin >> f;
			
			/// Really we should split it into three right here
			f.quadize();
		}
		
		else {
			string str;
			getline(cin, str);
			cout << str << endl;
			continue;
		}
		
	}
	
	return 0;
}

/// Important!  v1 has to come before v2 in CCW order!
Vertex *findEither(Vertex *v1, Vertex *v2)
{
	std::pair<Vertex*, Vertex*> p(v1, v2);
	
	map<pair<Vertex*,Vertex*>,Vertex*>::iterator it;
	
	it = addedVertices.find(p);
	
	if (it != addedVertices.end()) {
		return it->second;
	}
	
	std::pair<Vertex*, Vertex*> q(v2, v1);
	
	it = addedVertices.find(q);
	
	if (it != addedVertices.end()) {
		return it->second;
	}
    
	/// If we got here, it hasn't been added, so we create & add now
	Vertex v;
	v = *v2 - *v1;
	v *= 0.5;
	v = *v1 + v;
	    
	vlist.push_back(v);
    
    addedVertices[p] = &vlist.back();
	
	cout << vlist.back() << endl;
	
	return &vlist.back();
}

void Face::quadize()
{
	Vertex *center = centroid();

	/// We got it in CCW order I assume so we need to make sure we keep it!

	Vertex *v2v1 = findEither(V1, V2);
	Vertex *v3v2 = findEither(V2, V3);
	Vertex *v1v3 = findEither(V3, V1);
	
	Quad q1;
	q1.W = V1;
	q1.X = v2v1;
	q1.Y = center;
	q1.Z = v1v3;
	
	Quad q2;
	q2.W = V2;
	q2.X = v3v2;
	q2.Y = center;
	q2.Z = v2v1;
	
	Quad q3;
	q3.W = V3;
	q3.X = v1v3;
	q3.Y = center;
	q3.Z = v3v2;
	
	cout << q1 << "\n";
	qlist.push_back(q1);
	cout << q2 << "\n";
	qlist.push_back(q2);
	cout << q3 << "\n";
	qlist.push_back(q3);
}

Vertex * Face::centroid()
{
	Vertex ret;
	
	double x = V1->x + V2->x + V3->x;
	x /= 3.0;
	
	double y = V1->y + V2->y + V3->y;
	y /= 3.0;
	
	double z = V1->z + V2->z + V3->z;
	z /= 3.0;
	
	ret.x = x;
	ret.y = y;
	ret.z = z;
	
	vlist.push_back(ret);
	cout << vlist.back() << endl;
	return &vlist.back();
}

Vertex operator-(Vertex v1, Vertex v2)
{
	Vertex ret;
	ret.x = v1.x - v2.x;
	ret.y = v1.y - v2.y;
	ret.z = v1.z - v2.z;
	return ret;
}

Vertex operator+(Vertex v1, Vertex v2)
{
	Vertex ret;
	ret.x = v1.x + v2.x;
	ret.y = v1.y + v2.y;
	ret.z = v1.z + v2.z;
	return ret;
}

istream &operator>>(istream &is, Vertex &v)
{
	char c;
	double tx, ty, tz;
	
	is >> c;
	assert(c == 'v');
	is >> tx >> ty >> tz;
	v.x = tx;
	v.y = ty;
	v.z = tz;
	string trash;
	getline(is, trash);
	
	return is;
}

ostream &operator<<(ostream &os, Vertex &v)
{
    if (!v.id) {
        os << "v " << v.x << " " << v.y << " " << v.z;
        v.id = ++Vertex::vertex_id;
    }
	return os;
}

istream &operator>>(istream &is, Face &f)
{
	char c;
	unsigned int tx, ty, tz;
	
	is >> c;
	assert(c == 'f');
	is >> tx >> ty >> tz;
	f.v1 = tx;
	f.v2 = ty;
	f.v3 = tz;
	string trash;
	getline(is, trash);
	
	f.V1 = &vlist[f.v1];
	f.V2 = &vlist[f.v2];
	f.V3 = &vlist[f.v3];
	
	return is;
}

ostream &operator<<(ostream &os, Face &f)
{
	os << "f " << f.v1 << " " << f.v2 << " " << f.v3;
	return os;
}

ostream &operator<<(ostream &os, Quad &q)
{
	assert(q.W->id);
	assert(q.X->id);
	assert(q.Y->id);
	assert(q.Z->id);
	
	os << "f " << q.W->id << " " << q.X->id << " " << q.Y->id << " " << q.Z->id;
	
	return os;
}
