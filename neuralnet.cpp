
#include <iostream>
#include <cmath>
#include <functional>

#include "utility.h"
#include "neuralnet.h"
#include "dataset.h"

using namespace Eigen;

NeuralNet::NeuralNet(DataSet& data)
{

    //init the constants
    d = data.TrainingData.rows();
    n = data.inputs;
    h = data.hiddenNodes;
    m = data.TrainingData.cols() - n;

    //set the array sizes for feed forward
    X.resize(d,n);
    Xn.resize(d,n);
    Xb.resize(d,n+1);
    V.resize(n+1,h);
    H.resize(d,h);
    Hb.resize(d,h+1);
    W.resize(h+1,m);
    Y.resize(d,m);
    T.resize(d,m);

    //set the array sizes for backProp
    Wd.resize(h+1,m);
    Hd.resize(d,h+1);
    Hdnb.resize(d,h);
    
    //initilize the targets
    T<<data.TrainingData.rightCols(m);
    IF_DEBUG debugPrint(T, "T-targets");

    //initialize the input
    X<<data.TrainingData.leftCols(data.inputs);
    IF_DEBUG debugPrint(X,"X-inputs");

    //normalize inputs
    Xn = normMinMax(X);
    IF_DEBUG debugPrint(Xn,"Xn-inputs normalized");

    //add bias to x of -1
    Xb<<VectorXd::Constant(d,-1.0),Xn.leftCols(n);
    IF_DEBUG debugPrint(Xb, "Xb-input with bias");

    //initialize the weight matrices
    V=randUnif(n+1,h,0.0,1.0);
    W=randUnif(h+1,m,0.0,1.0);
    IF_DEBUG debugPrint(V, "V-weights");
    IF_DEBUG debugPrint(W, "W-weights");
}

NeuralNet::~NeuralNet()
{

}

//public functions
void NeuralNet::train(double eta, double num_iter)
{
    for(int i = 0; i < num_iter; i++)
    {
        feedForward(X);
        backProp(eta);
        if(DEBUG)
        {
            if( i % 1000 == 0)
            {
            printf("---Gen %d ---\n",i);
            debugPrint(X,"Input");
            debugPrint(Y.unaryExpr([](double x){return x >= 0.5 ? 1.0 : 0.0;}),"Y");
            debugPrint(T,"T");
            }
        }
    }
    csclassPrint(T,"Target");
    csclassPrint(Y.unaryExpr([](double x){return x >= 0.5 ? 1.0 : 0.0;}),"Predicted");
}

Eigen::VectorXd NeuralNet::predict(const Eigen::VectorXd &input)
{   
    Eigen::VectorXd output(5);
    return output;
}

//private functions

double NeuralNet::transfer(double x)
{
  return 1.0/(1.0 + std::exp(-4.0 * x));
}

void NeuralNet::feedForward(const Eigen::MatrixXd &input)
{
    //apply the weights to the connects Xb->H
    H = (Xb*V).unaryExpr([this](double x){ return NeuralNet::transfer(x);});
    IF_DEBUG debugPrint(H,"H-Hidden nodes");

    Hb<<VectorXd::Constant(d,-1.0),H.leftCols(h);
    IF_DEBUG debugPrint(Hb, "Hb-hidden nodes with bias");
    IF_DEBUG debugPrint(W, "W - Weights");
    IF_DEBUG debugPrint(Hb*W, "Hb*W - Weights");
    //calculuate t he outputs
    Y = (Hb*W).unaryExpr([this](double x){ return NeuralNet::transfer(x);});
    IF_DEBUG debugPrint(Y, "Y-outputs");   
}

void NeuralNet::backProp(double eta)
{

    //make some temp matrices
    MatrixXd t_1;
    MatrixXd t_2;
    //compute wd = (y-t) * y (1-y)
    t_1 = Y-T;
    IF_DEBUG debugPrint(t_1, "Y-T");
    t_2 = MatrixXd::Constant(d,m,1.0) - Y;
    IF_DEBUG debugPrint(t_2, "1-Y");

    Wd = t_1.array()*Y.array()*t_2.array();
    IF_DEBUG debugPrint(Wd, "Wd = (Y-T) * Y * (1.0 - Y)");

    //compute Hw = Hb*(1-hb)*(Wd Wt)
    t_1 = MatrixXd::Constant(d,h+1,1.0) - Hb;
    IF_DEBUG debugPrint(t_1, "1-Hb");
    t_2 =Wd*W.transpose();
    IF_DEBUG debugPrint(t_2, "Wd W.transpose");

    Hd = Hb.array() * (t_1).array()*(t_2).array();
    IF_DEBUG debugPrint(Hd, "Hd = Hb*(1-Hb)*(Wd W.transpose");

    //W -= eta *(Hb.trans Wd)
    W -= (eta*(Hb.transpose() * Wd).array()).matrix();
    IF_DEBUG debugPrint(W, "W -= eta*(Hb.trans Wd)");

    //subtract of the bias now
    Hdnb << Hd.rightCols(h);
    IF_DEBUG debugPrint(Hdnb, "Hdnb is right h col of Hd");

    //V-=eta *(Xb.trans Hdnb)
    V -= (eta*(Xb.transpose() *Hdnb).array()).matrix();
    IF_DEBUG debugPrint(V, "V-=eta *(Xb.trans Hdnb)");
}
