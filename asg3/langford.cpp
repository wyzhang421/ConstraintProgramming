#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/minimodel.hh>
#include <gecode/driver.hh>
#include <cstdlib>

using namespace Gecode;
using namespace std;


int result[100];


//Definition of class MyOptions
class MyOptions : public Options {
	
public:
	int n,k;
	
	// Initialize options
	MyOptions(const char*s, int n0, int k0): Options(s), n(n0), k(k0){}
	
	//Parse options from arguments
	void parse(int& argc, char* argv[]){
		Options::parse(argc,argv);
		//cout<<"in class"<<argv[1]<<endl;
		n = atoi(argv[1]);
		k = atoi(argv[2]);
	}
};


class langford : public Script {
protected:
	IntVarArray x;
	IntVarArray y;
	const MyOptions& opt;
public:

    enum{
		MODEL_ONE,
		MODEL_TWO,
		MODEL_THREE,
		MODEL_FOUR,
		SEARCH_ONE,
		SEARCH_TWO,
		SEARCH_THREE
	};

    langford(const MyOptions& _opt) : Script(opt), opt(_opt), x(*this, _opt.n*_opt.k, 1, _opt.n*_opt.k) ,y(*this, _opt.n*_opt.k, 0, _opt.n*_opt.k-1) {   
		int n = _opt.n;
		int k = _opt.k;
		switch(opt.model()){

			case MODEL_ONE:

				distinct(*this, x);
				for(int i = 0; i <= n-1; ++i)
					for(int j=1; j <= k-1; ++j)
						rel(*this, x[i*k+j] == x[i*k+j-1]+i+2);
				branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
			break;

			case MODEL_TWO:
				distinct(*this, y, ICL_DOM);

				// implication constraint
				for(int i = 0; i <= n - 1; ++i )
					for(int j = 1; j <= n*k -(k-1)*(i+2); ++j )
						for(int c = 1; c <= k-1; ++c )
						{
							BoolVar b = expr(*this, y[j-1]==i*k);
							IntVar r (*this, i*k+c, i*k+c );
							ite(*this, b, y[j+c*(i+2)-1], r, r);
						}

				for(int i=0; i <= n-1; ++i)
					for(int j = n*k -(k-1)*(i+2)+1; j <= n*k; ++j)
						rel(*this, y[j-1] != i*k);
				branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
			break;
			
			case MODEL_THREE:

				distinct(*this, x);
				for(int i = 0; i <= n-1; ++i)
					for(int j=1; j <= k-1; ++j)
						rel(*this, x[i*k+j] == x[i*k+j-1]+i+2);

				distinct(*this, y, ICL_DOM);	
				for(int i = 0; i <= n - 1; ++i )
					for(int j = 1; j <= n*k -(k-1)*(i+2); ++j )
						for(int c = 1; c <= k-1; ++c )
						{
							BoolVar b = expr(*this, y[j-1]==i*k);
							IntVar r (*this, i*k+c, i*k+c );
							ite(*this, b, y[j+c*(i+2)-1], r, r);
						}
				
				for(int i=0; i <= n-1; ++i)
					for(int j = n*k -(k-1)*(i+2)+1; j <= n*k; ++j)
						rel(*this, y[j-1] != i*k);
				
				channel(*this, x, 1, y, 0);
				
				//Definition of method of branching
				switch(opt.search()){
					
					// only branch x
					case SEARCH_ONE:
						branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
					
					// only branch y
					case SEARCH_TWO:
						branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
					
					//branch both x and y
					case SEARCH_THREE:
						branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
						branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
				}
			break;	
			
			
			
			case MODEL_FOUR:
				distinct(*this, x);
				for(int i = 0; i <= n-1; ++i)
					for(int j=1; j <= k-1; ++j)
						rel(*this, x[i*k+j] == x[i*k+j-1]+i+2);

				distinct(*this, y, ICL_DOM);	
				for(int i = 0; i <= n - 1; ++i )
					for(int j = 1; j <= n*k -(k-1)*(i+2); ++j )
						for(int c = 1; c <= k-1; ++c )
						{
							BoolVar b = expr(*this, y[j-1]==i*k);
							IntVar r (*this, i*k+c, i*k+c );
							ite(*this, b, y[j+c*(i+2)-1], r, r);
						}
						
				for(int i=0; i <= n-1; ++i)
					for(int j = n*k -(k-1)*(i+2)+1; j <= n*k; ++j)
						rel(*this, y[j-1] != i*k);
				
				channel(*this, x, 1, y, 0);
				
				rel(*this, x[0] <= (n*k-1)/2 );

				//Definition of method of branching
				switch(opt.search()){
					// only branch x
					case SEARCH_ONE:
						branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
					
					// only branch y
					case SEARCH_TWO:
						branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
					
					//branch both x and y
					case SEARCH_THREE:
						branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
						branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
					break;
				}
			break;	
		}
    }

    // search support
    langford(bool share, langford& s) : Script(share, s), opt(s.opt) {
		switch(opt.model()){
			case MODEL_ONE:
				x.update(*this, share, s.x);
			break;
			case MODEL_TWO:
				y.update(*this, share, s.y);
			break;
			
			case MODEL_THREE:
				x.update(*this, share, s.x);
				y.update(*this, share, s.y);
			break;
			
			case MODEL_FOUR:
				x.update(*this, share, s.x);
				y.update(*this, share, s.y);
			break;
		}
    }
    virtual Script* copy(bool share) {
		return new langford(share,*this);
    }

  // print solution
	void print(std::ostream& os) const {     
		switch(opt.model()){
			
			case MODEL_ONE:
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					result[x[i].val()-1] = i/opt.k +1;
					
				}
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					cout<< result[i] <<" ";
				}
				cout<<endl;
				//std::cout << x << std::endl;
			break;
			
			case MODEL_TWO:
				//std::cout << y << std::endl;
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					result[i] = y[i].val()/opt.k+1;
				}
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					cout<< result[i] <<" ";
				}
				cout<<endl;
			break;
		
			case MODEL_THREE:
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					result[x[i].val()-1] = i/opt.k +1;
					
				}
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					cout<< result[i] <<" ";
				}
				cout<<endl;
				//std::cout << x << std::endl;
				//std::cout << y << std::endl;
			break;
			
			case MODEL_FOUR:
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					result[x[i].val()-1] = i/opt.k +1;
					
				}
				for(int i=0; i<opt.n*opt.k; ++i)
				{
					cout<< result[i] <<" ";
				}
				cout<<endl;
				//std::cout << x << std::endl;
				//std::cout << y << std::endl;
			break;
		}
	}
};


// main function
int main(int argc, char* argv[]) {
	
	MyOptions opt("Langford",1,1);
  
	opt.model(langford::MODEL_ONE, "1", "use model one");
	opt.model(langford::MODEL_TWO, "2", "use model two");
	opt.model(langford::MODEL_THREE, "3", "use the channelling");
	opt.model(langford::MODEL_FOUR, "4", "use symmetry breaking");
	opt.model(langford::MODEL_ONE);
	  
	  
	opt.search(langford::SEARCH_ONE, "1", "searching model one variables");
	opt.search(langford::SEARCH_TWO, "2", "searching model two variables");
	opt.search(langford::SEARCH_THREE, "3", "searching model both models variables");
  
	opt.solutions(0);
	opt.parse(argc, argv);

	Script::run<langford, DFS, MyOptions>(opt);

	return 0;
}


/*
plz use these commands to compile and link

setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c <dir>/langford.cpp
g++ -o langford -L<dir>/lib langford.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist
./langford -model 2 4 2
*/


