#include <iostream>
#include "Eigen/Dense"
#include "dataset.h"

using namespace Eigen;

int main()
{
  /*
  MatrixXd m(2,2);
  m.setRandom();
  m(1,1) = m(1,0) + m(0,1);
  std::cout << m << std::endl;
  */
 DataSet newor;
 newor.print();
 return 0;
}