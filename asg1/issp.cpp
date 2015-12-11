#include<gecode/int.hh>
#include<gecode/search.hh>
#include<gecode/driver.hh>

using namespace Gecode;

//structure of dataxx[]
//the first line: n--the given orders, w--size of master square
//the second line: the size sequence of all small squares
//by using this structure, the program can test other cases.
const int data00[] = {
   13, 23,
   1,1,2,2,3,3,4,5,5,7,11,11,12
};

const int* s = &data00[0];

class issp: public Script{
protected:
	IntVarArray x;
	IntVarArray y;
	const Options& opt;
public:

	enum{
		MODEL_ONE,
		MODEL_TWO
	};
	
	issp(const Options& _opt) 
	  : Script(opt),opt(_opt),
		x(*this,s[0],0,s[1]-1),
		y(*this,s[0],0,s[1]-1){
		
		int n = *s++;  //the given orders
		int w = *s++;  //size of master square
		
		IntArgs size(n,s);  //the size sequence of all small squares

		// Restrict position according to square size
		for (int i=0; i<13; i++ ) {
			rel(*this, x[i], IRT_LQ, w-size[i]);
		    rel(*this, x[i], IRT_LQ, w-size[i]);
		}

		//Capacity constraints-- Well Covered Constraints
		{
			for (int cx=0; cx<w; cx++) {
				BoolVarArgs bx(*this,n,0,1);
				for (int i=0; i<n; i++)
					dom(*this, x[i], cx-size[i]+1, cx, bx[i]);
				linear(*this, size, bx, IRT_EQ, w);
			}
			for (int cy=0; cy<w; cy++) {
				BoolVarArgs by(*this,n,0,1);
				for (int i=0; i<n; i++)
					dom(*this, y[i], cy-size[i]+1, cy, by[i]);
				linear(*this, size, by, IRT_EQ, w);
			}
		}

		// No Overlap Constraints
		switch(opt.model()){

			//MODEL_ONE uses nooverlap function
			case MODEL_ONE:
				nooverlap(*this,x,size,y,size);
			    break;

			//MODEL_TWO does not use nooverlap function
			case MODEL_TWO:{
				for(int i=0; i<n-1; i++)
					for(int j=i+1; j<n; j++)
						rel(*this, (x[i]+size[i]<=x[j]) || (x[j]+size[j]<=x[i]) || (y[i]+size[i]<=y[j]) || (y[j]+size[j]<=y[i]));
				}
				break;
		}

		//post branching
		branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}

	//search support
	issp(bool share, issp& s): Script(share, s), opt(s.opt){
		x.update(*this, share, s.x);
		y.update(*this, share, s.y);
	}

	virtual Script* copy(bool share){
		return new issp(share, *this);
	}

	void print(std::ostream& os) const{
		for (int i=0; i<x.size(); i++)
			os << x[i] << " " << y[i] << std::endl;
	}

};

int main(int argc, char* argv[]){


	Options opt("Imperfect Squared Square Placement");
	opt.model(issp::MODEL_ONE, "one", "use the frist model");
	opt.model(issp::MODEL_TWO, "two", "use the second model");
	opt.model(issp::MODEL_ONE);
	opt.solutions(1);
	opt.parse(argc, argv);

	Script::run<issp, DFS, Options>(opt);

	return 0;
}



/*

please use the following commands for compiling & linking

setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c issp.cpp
g++ -o issp -L<dir>/lib issp.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist


Output:

linux1:/uac/msc/wyzhang> ./issp -model one
Imperfect Squared Square Placement
11 10
19 7
12 10
18 5
11 7
20 5
19 8
14 7
18 0
11 0
0 0
12 12
0 11

Initial
        propagators: 644
        branchers:   2

Summary
        runtime:      0.047 (47.888 ms)
        solutions:    1
        propagations: 525556
        nodes:        3691
        failures:     1838
        restarts:     0
        no-goods:     0
        peak depth:   24

linux1:/uac/msc/wyzhang> ./issp -model two
Imperfect Squared Square Placement
11 10
19 7
12 10
18 5
11 7
20 5
19 8
14 7
18 0
11 0
0 0
12 12
0 11

Initial
        propagators: 1033
        branchers:   2

Summary
        runtime:      0.636 (636.137 ms)
        solutions:    1
        propagations: 5952112
        nodes:        20943
        failures:     10464
        restarts:     0
        no-goods:     0
        peak depth:   39

*/

