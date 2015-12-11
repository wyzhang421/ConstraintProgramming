#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/minimodel.hh>
#include <gecode/driver.hh>

using namespace Gecode;

const int N=6;

//test whether target can be a legal move of source
bool test_support(int source, int target)
{
	static const int legalmoves[8][2] = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}};
	int t[8];
	int n_nbs = 0;
	int a = (source-1) / N;
	int b = (source-1) % N;
	
	//t[] indicates all possible legal move from source
	for(int i = 0; i < 8; ++i)
	{
		int nx = a + legalmoves[i][0];
		int ny = b + legalmoves[i][1];
		if( (nx >= 0) && (nx < N) && (ny >= 0) && (ny < N) )
		{
			t[n_nbs++] = nx * N + ny + 1;
		}	
	}
	//test whether target in t[]
	for(int i=0; i < n_nbs; ++i)
	{
		if(target==t[i])
		{
			return true;
		}
	
	}
	return false;
	
}



class Move : public Propagator {
protected:
	Int::IntView x0, x1;
public:
    // posting
    Move(Space& home, Int::IntView y0, Int::IntView y1) : Propagator(home), x0(y0), x1(y1) {
		x0.subscribe(home,*this,Int::PC_INT_DOM);
		x1.subscribe(home,*this,Int::PC_INT_DOM);
    }
    static ExecStatus post(Space& home, Int::IntView x0, Int::IntView x1) {
    (void) new (home) Move(home,x0,x1);
    return ES_OK;
    }
    // disposal
    virtual size_t dispose(Space& home) {
		x0.cancel(home,*this,Int::PC_INT_DOM);
		x1.cancel(home,*this,Int::PC_INT_DOM);
		(void) Propagator::dispose(home);
		return sizeof(*this);
    }
    // copying
    Move(Space& home, bool share, Move& p) : Propagator(home,share,p) {
		x0.update(home,share,p.x0);
		x1.update(home,share,p.x1);
    }
    virtual Propagator* copy(Space& home, bool share) {
		return new (home) Move(home,share,*this);
    }
    // cost computation
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
		return PropCost::binary(PropCost::LO);
    }
    // propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&)  {
		
		//modify domain of x0;
		for(int valuex0=1; valuex0 <= N * N; ++valuex0){
			int flag=0;
			for(int valuex1=1; valuex1 <= N * N; ++valuex1){
				if(x1.in(valuex1) && x0.in(valuex0)){
					if(test_support(valuex0,valuex1)==true) {
						flag=1;
						break;
					}
				}						
			}
			if(flag==0)	//value0 can  not find any support in D(X1)
			{
				if (x0.nq(home, valuex0) == Int::ME_INT_FAILED)
					return ES_FAILED;
			}
		}
		
			
		//modify domain of x1;
		for(int valuex1=1; valuex1 <= N * N; ++valuex1){
			int flag=0;
			for(int valuex0=1; valuex0 <= N * N; ++valuex0){
				if(x1.in(valuex1) && x0.in(valuex0)){
					if(test_support(valuex0,valuex1)==true) {
						flag=1;
						break;
					}
				}						
			}
			if(flag==0)	//value1 can  not find any support in D(X0)
			{
				if (x1.nq(home, valuex1) == Int::ME_INT_FAILED)
					return ES_FAILED;
			}
		}
	
		if (x0.assigned() && x1.assigned())
			return home.ES_SUBSUMED(*this);
		else 
			return ES_NOFIX;
	}
};

void move(Space& home, IntVar x0, IntVar x1, const int N) {
    // constraint post function
    Int::IntView y0(x0), y1(x1);
    if (Move::post(home,y0,y1) != ES_OK)
		home.fail();
}


class knight : public Space {
protected:
	IntVarArray x;
public:

	knight(void): x(*this, N*N, 1, N*N){
		
		//all_different constraint
		distinct(*this, x);

		//configuration of figure 2
		rel(*this, x[0]==1);
		rel(*this, x[8]==25);
		rel(*this, x[17]==23);
		rel(*this, x[26]==22);

		
		
		//move constraint
		for(int i = 0; i < N*N-1; i++){
			move(*this, x[i], x[i+1], N);
		}
		move(*this, x[N*N-1], x[0], N);
	

		//post branching
		branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}

	//search support
	knight(bool share, knight& s):Space(share, s){
		x.update(*this, share, s.x);
	}
	virtual Space* copy(bool share){
		return new knight(share, *this);
	}
	//print solution
	void print(void) const{
	
		std::cout<<x<<std::endl; 

	}
};

// main function
int main(int argc, char* argv[]) {
    // create model and search engine
    knight* s = new knight;
    DFS<knight> e(s);
    delete s;
  // search and print all solutions
    while (knight* s = e.next()) {
		s->print(); delete s;
    }
	return 0;
}

/*

please use the following commands for compiling & linking

setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c knight.cpp
g++ -o knight -L<dir>/lib knight.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist
./knight

chessboard labeling
1   2	3	4	5	6
7	8	9	10	11	12
13	14	15	16	17	18
19	20	21	22	23	24
25	26	27	28	29	30
31	32	33	34	35	36



output:
linux1:/uac/msc/wyzhang/CSP/asg2/asg2f> ./knight
{1, 9, 5, 18, 10, 6, 17, 21, 25, 33, 29, 16, 8, 19, 32, 28, 36, 23, 12, 4, 15, 2, 13, 26, 34, 30, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 5, 18, 10, 6, 17, 21, 25, 33, 29, 16, 8, 19, 32, 28, 36, 23, 12, 4, 15, 2, 13, 26, 34, 30, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 9, 13, 2, 10, 6, 17, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 32, 19, 15, 26, 34, 30, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 13, 2, 10, 6, 17, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 32, 19, 15, 26, 34, 30, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 9, 17, 6, 10, 2, 13, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 32, 19, 15, 26, 34, 30, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 17, 6, 10, 2, 13, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 32, 19, 15, 26, 34, 30, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 9, 5, 16, 12, 4, 8, 21, 25, 33, 29, 18, 10, 6, 17, 30, 34, 23, 36, 28, 32, 19, 15, 2, 13, 26, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 5, 16, 8, 19, 32, 21, 25, 33, 29, 18, 10, 6, 17, 4, 12, 23, 36, 28, 20, 31, 27, 35, 24, 11, 22, 30, 34, 26, 13, 2, 15, 7, 3, 14}
{1, 9, 5, 16, 8, 19, 32, 21, 25, 33, 29, 18, 10, 6, 17, 4, 12, 23, 36, 28, 15, 2, 13, 26, 34, 30, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 5, 16, 8, 19, 32, 21, 25, 33, 29, 18, 10, 6, 17, 28, 36, 23, 12, 4, 15, 2, 13, 26, 34, 30, 22, 11, 24, 35, 27, 31, 20, 7, 3, 14}
{1, 9, 5, 16, 12, 4, 8, 21, 25, 33, 29, 18, 10, 6, 17, 30, 34, 23, 36, 28, 32, 19, 15, 2, 13, 26, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 9, 5, 16, 8, 19, 32, 21, 25, 33, 29, 18, 10, 6, 17, 4, 12, 23, 36, 28, 15, 2, 13, 26, 34, 30, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 9, 5, 16, 8, 19, 32, 21, 25, 33, 29, 18, 10, 6, 17, 28, 36, 23, 12, 4, 15, 2, 13, 26, 34, 30, 22, 35, 24, 11, 3, 7, 20, 31, 27, 14}
{1, 14, 3, 7, 15, 19, 32, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 20, 31, 27, 35, 24, 11, 22, 26, 34, 30, 17, 6, 10, 2, 13, 9}
{1, 14, 3, 7, 15, 19, 32, 21, 25, 33, 29, 18, 5, 16, 8, 4, 12, 23, 36, 28, 20, 31, 27, 35, 24, 11, 22, 30, 34, 26, 13, 2, 10, 6, 17, 9}

*/










