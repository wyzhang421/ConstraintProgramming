#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
 
 using namespace Gecode;


 
class Queens : public Script {
public:
   IntVarArray q;
   const SizeOptions& opt;
   enum{
		MODEL_ONE,  
		MODEL_TWO,   
		MODEL_THREE
   };
   
   
   Queens(const SizeOptions& _opt) : Script(opt), opt(_opt), q(*this, _opt.size()*2-1, -_opt.size(), _opt.size()*2-1) 
   {
	    const int N = opt.size();
		
	    // domain constraint
	    for(int i=0; i < q.size(); ++i)
	    {
			std::vector<int> a;
			a.push_back(-N);
			int k = abs(i-(N-1));
			for(int m=1; k+m <= 2*N-k-1; m=m+2)
			{
				a.push_back(k+m);
			}
			IntSet d(&a[0],a.size());
			//dom(*this, q[i], IntSet(r,a.size()));
			q[i] = IntVar(*this, IntSet(&a[0],a.size()));
			//std::cout<<q[i]<<std::endl;
	    }
	   
	   
	   //count constraint1
	    IntSet temp(1,2*N-1);
	    count(*this, q, temp, IRT_EQ, N);
		
	   
	   //count constraint2
	    for(int i=1; i <= q.size(); ++i)
		    count(*this, q, i, IRT_LQ, 1);
	   
	   
	    switch(opt.model()){
			// arthimatic constraint
		    case MODEL_ONE:
				for(int i=0; i < q.size()-1; ++i)
					for(int j=i+1; j < q.size(); ++j)
					{
						rel(*this, abs(q[i]-q[j]) != abs(i-j));
					}
		    break;
			
		   // tuple-set
		    case MODEL_TWO: 
			{
				for(int i=0; i < q.size()-1; ++i)
					for(int j=i+1; j < q.size(); ++j)
					{
						TupleSet table1;
						IntVarArray l(*this, 2);
						for(IntVarValues tmpi(q[i]); tmpi(); ++tmpi)
							for(IntVarValues tmpj(q[j]); tmpj(); ++tmpj)
								if( abs(tmpi.val()-tmpj.val()) != abs(i-j))
								{
									table1.add(IntArgs(2,tmpi.val(),tmpj.val()));
	
									l[0]=q[i];
									l[1]=q[j];
								}
						table1.finalize();
						extensional(*this, l, table1);
					}
		    }
		    break;
		    
			//DFA
		    case MODEL_THREE:
			{
				int f[2]={100,-1};
				for(int i=0; i < q.size()-1; ++i)
				{
					for(int j=i+1; j < q.size(); ++j)
					{
						
						int c1=0,c2=0; // c1(c2) is the number of values in y1's(y2's) domain
						for(IntVarValues tmpi(q[i]); tmpi(); ++c1,++tmpi);
						for(IntVarValues tmpj(q[j]); tmpj(); ++c2,++tmpj);	
	
						DFA:: Transition t[c1+c1*c2+1];
						int m=0, k=c1;
						for(IntVarValues tmpi(q[i]); tmpi(); ++m,++tmpi)
						{	
							t[m].i_state=0;
							t[m].symbol=tmpi.val();
							t[m].o_state=m+1;
							for(IntVarValues tmpj(q[j]); tmpj(); ++k,++tmpj)
							{
								t[k].i_state=m+1;
								t[k].symbol=tmpj.val();
								if(abs(tmpi.val()-tmpj.val()) != abs(i-j))
								{
									t[k].o_state=100;
								}
								else
								{
									t[k].o_state=-1;
								}
							}
						}
						t[c1+c1*c2].i_state=-1;
						t[c1+c1*c2].symbol=0;
						t[c1+c1*c2].o_state=0;


						IntVarArray l(*this, 2,-9,9);
						DFA d(0, t, f);
						l[0]=q[i];
						l[1]=q[j];
						extensional(*this, l, d);
					}
				}
			}
		    break;
	    }   
		
		branch(*this, q, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
   }
   
   
   //search support
   Queens(bool share, Queens &s) : Script(share, s), opt(s.opt){
	   q.update(*this, share, s.q);
   }
   
   virtual Script* copy(bool share){
	   return new Queens(share, *this);
   }
   
   void print(std::ostream& os) const{
	 std::cout << q <<std::endl;
   }
 
};



int main(int argc, char* argv[])
{
	
	SizeOptions opt("N-Queens");
	opt.model(Queens::MODEL_ONE, "1", "use arithmetic constraint");
	opt.model(Queens::MODEL_TWO, "2", "use TupleSet");
	opt.model(Queens::MODEL_THREE, "3", "use DFA");
	opt.size(2);
	opt.solutions(0);
	opt.parse(argc,argv);
	
	Script:: run<Queens, DFS, SizeOptions>(opt);
	
	return 0;
	
}



/*

plz use following command to compile:
setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c nqueens.cpp
g++ -o nqueens -L<dir>/lib nqueens.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist
./nqueens -model 1 5
./nqueens -model 2 5
./nqueens -model 3 5




output:
linux1:/uac/msc/wyzhang/CSP/asg2/asg2f> ./nqueens -model 1 5
N-Queens
{-5, -5, 3, 6, 9, 2, 5, -5, -5}
{-5, -5, 5, 8, 1, 4, 7, -5, -5}
{-5, -5, 5, 2, 9, 6, 3, -5, -5}
{-5, -5, 7, 4, 1, 8, 5, -5, -5}
{-5, 4, -5, 8, 5, 2, -5, 6, -5}
{-5, 6, -5, 2, 5, 8, -5, 4, -5}
{-5, 4, 7, -5, 3, 6, -5, -5, 5}
{-5, 6, 3, -5, 7, 4, -5, -5, 5}
{5, -5, -5, 4, 7, -5, 3, 6, -5}
{5, -5, -5, 6, 3, -5, 7, 4, -5}

Initial
        propagators: 80
        branchers:   1

Summary
        runtime:      0.002 (2.969 ms)
        solutions:    10
        propagations: 15113
        nodes:        285
        failures:     133
        restarts:     0
        no-goods:     0
        peak depth:   11

linux1:/uac/msc/wyzhang/CSP/asg2/asg2f> ./nqueens -model 2 5
N-Queens
{-5, -5, 3, 6, 9, 2, 5, -5, -5}
{-5, -5, 5, 2, 9, 6, 3, -5, -5}
{-5, -5, 5, 8, 1, 4, 7, -5, -5}
{-5, -5, 7, 4, 1, 8, 5, -5, -5}
{-5, 4, -5, 8, 5, 2, -5, 6, -5}
{-5, 6, -5, 2, 5, 8, -5, 4, -5}
{-5, 4, 7, -5, 3, 6, -5, -5, 5}
{-5, 6, 3, -5, 7, 4, -5, -5, 5}
{5, -5, -5, 4, 7, -5, 3, 6, -5}
{5, -5, -5, 6, 3, -5, 7, 4, -5}

Initial
        propagators: 44
        branchers:   1

Summary
        runtime:      0.009 (9.798 ms)
        solutions:    10
        propagations: 2503
        nodes:        63
        failures:     22
        restarts:     0
        no-goods:     0
        peak depth:   7

linux1:/uac/msc/wyzhang/CSP/asg2/asg2f> ./nqueens -model 3 5
N-Queens
{-5, -5, 3, 6, 9, 2, 5, -5, -5}
{-5, -5, 5, 2, 9, 6, 3, -5, -5}
{-5, -5, 5, 8, 1, 4, 7, -5, -5}
{-5, -5, 7, 4, 1, 8, 5, -5, -5}
{-5, 4, -5, 8, 5, 2, -5, 6, -5}
{-5, 6, -5, 2, 5, 8, -5, 4, -5}
{-5, 4, 7, -5, 3, 6, -5, -5, 5}
{-5, 6, 3, -5, 7, 4, -5, -5, 5}
{5, -5, -5, 4, 7, -5, 3, 6, -5}
{5, -5, -5, 6, 3, -5, 7, 4, -5}

Initial
        propagators: 44
        branchers:   1

Summary
        runtime:      0.003 (3.319 ms)
        solutions:    10
        propagations: 1837
        nodes:        63
        failures:     22
        restarts:     0
        no-goods:     0
        peak depth:   7

*/















