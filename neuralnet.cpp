
#include <iostream>
#include <cmath>
#include <functional>

#include "utility.h"
#include "neuralnet.h"
#include "dataset.h"

using namespace Eigen;

NeuralNet::NeuralNet(DataSet& data, double V_lower, double V_upper, double W_lower, double W_upper)
{
    //init number of classes
    numOfClasses = data.numOfClasses;

    //init testing constants
    test_d = data.TestData.rows();
    test_n = data.inputs;
    test_m = data.TestData.cols() - test_n;

    //init the constants for training
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
    
    //set the array sizes for test data 
    test_X.resize(test_d,test_n);
    test_Xn.resize(test_d,test_n);
    test_Xb.resize(test_d,test_n+1);
    test_H.resize(test_d,h);
    test_Hb.resize(test_d,h+1);
    test_T.resize(test_d,test_m);
    test_Y.resize(test_d,test_m);

    conf.resize(test_d,test_m);

    //initilize the targets
    T<<data.TrainingData.rightCols(m);
    IF_DEBUG debugPrint(T, "T-targets");

    test_T<<data.TestData.rightCols(test_m);
    IF_DEBUG debugPrint(test_T, " test_T-targets");

    //initialize the input
    X<<data.TrainingData.leftCols(data.inputs);
    IF_DEBUG debugPrint(X,"X-inputs");

    test_X<<data.TestData.leftCols(data.inputs);
    IF_DEBUG debugPrint(test_X,"test_X-inputs");

    //normalize inputs
    Xn = normMinMax(X);
    IF_DEBUG debugPrint(Xn,"Xn-inputs normalized");

    test_Xn = normMinMax(test_X);
    IF_DEBUG debugPrint(test_Xn,"test_Xn-inputs normalized");
    
    //add bias to x of -1
    Xb<<VectorXd::Constant(d,-1.0),Xn.leftCols(n);
    IF_DEBUG debugPrint(Xb, "Xb-input with bias");

    test_Xb<<VectorXd::Constant(test_d,-1.0),test_Xn.leftCols(test_n);
    IF_DEBUG debugPrint(test_Xb, "Xb-input with bias");

    //initialize the weight matrices
    V=randUnif(n+1,h,V_lower,V_upper);
    W=randUnif(h+1,m,V_lower,V_upper);
    IF_DEBUG debugPrint(V, "V-weights");
    IF_DEBUG debugPrint(W, "W-weights");
}

//default for use with auto setting of weight range

NeuralNet::~NeuralNet()
{

}

//public functions
void NeuralNet::train(double eta, double num_iter, bool python_check)
{   //added code for printing csv
    //version
    if(CSV_PRINT)
    {
        //print of X colllums for csv based on # of inputs
        for(int i = 0; i < n; i++)
        {
            printf("X[%d],",i);
        }
           //print of X colllums for csv based on # of inputs
        for(int i = 0; i < n; i++)
        {
            printf("Xn[%d],",i);
        }
        for(int r = 0; r < n+1; r++)
        {
            for(int c = 0; c < h; c++)
            {
                //V(weight) for synapse from [neuron] to [hidden node] n+1 node is bias
                printf("V[%d][%d],",r,c);
            }
        }
        for(int i = 0; i < h; i++)
        {
            printf("H[%d],",i);
        }
        for(int i = 0; i < h; i++)
        {
            printf("t(H[%d]),",i);
        }
        for(int r = 0; r < h+1; r++)
        {
            for(int c = 0; c < m; c++)
            {
                //V(weight) for synapse from [neuron] to [hidden node] n+1 node is bias
                printf("W[%d][%d],",r,c);
            }
        }
        for(int i = 0; i < m; i++)
        {
            printf("Y[%d]),",i);
        }
        for(int i = 0; i < m; i++)
        {
            printf("t(Y[%d])),",i);
        }
        for(int i = 0; i < m; i++)
        {
            printf("max(t(Y[%d]))),",i);
        }
        for(int i = 0; i < m; i++)
        {
            if(i != m-1)
            {
                printf("T[%d],",i);
            }
            else
            {
                printf("T[%d]",i);
            }
            
        }
        printf("\n");

    }
    for(int i = 0; i < num_iter; i++)
    {
        feedForward(X);
        //print out stuff for csv every CSV_PRINTEVERY iterations
        if(CSV_PRINT && (i%CSV_PRINTEVERY)==0)
        {
            csvPrint(X,"X");
            csvPrint(Xn,"Xn");
            csvWeightPrint(V,n,h);
            csvPrint(Xb*V,"H pree");
            csvPrint(H,"H post transfer");
            csvWeightPrint(W,h,m);
            csvPrint(Hb*W);
            csvPrint(Y);
            csvPrint(Y.unaryExpr([](double x){return x >= 0.5 ? 1.0 : 0.0;}));
            csvPrint(T,"targets",true);
            printf("\n");
        }
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
    if(python_check)
    {
        std::cout<<RMSE(T, Y.unaryExpr([](double x){return x >= 0.5 ? 1.0 : 0.0;}))<<',';
    }
}

void NeuralNet::predict(int nnoneof, bool python_check)
{   
    //apply the weight matricies

    //apply the weights to the connects Xb->H
    test_H = (test_Xb*V).unaryExpr([this](double x){ return NeuralNet::transfer(x);});
    IF_DEBUG debugPrint(test_H,"test_H-Hidden nodes");

    test_Hb<<VectorXd::Constant(test_d,-1.0),test_H.leftCols(h);
    IF_DEBUG debugPrint(Hb, "Hb-hidden nodes with bias");
    IF_DEBUG debugPrint(W, "W - Weights");
    IF_DEBUG debugPrint(test_Hb*W, "test_Hb*W - Weights");
    //calculuate t he outputs
    test_Y = (test_Hb*W).unaryExpr([this](double x){ return NeuralNet::transfer(x);});
    IF_DEBUG debugPrint(test_Y, "test_Y-outputs");   

    //now calc confusion
    //if nonof then make temp_Y argmax of each row
    MatrixXd temp_Y;

    if(nnoneof == 0)
    {
        temp_Y = test_Y.unaryExpr([](double x){return x >= 0.5 ? 1.0 : 0.0;});
    }
    else
    {
        //VectorXd::Index index;
        for(int r = 0; r < test_d; r++)
        {
            temp_Y.resize(test_d,1);
            MatrixXf::Index   maxIndex;
            test_Y.row(r).maxCoeff( &maxIndex);
            temp_Y(r,0) = double(maxIndex);
        }
    }
    //std::cout<<RMSE(test_T,temp_Y);
    if(!python_check && !CSV_PRINT)
    {
        csclassPrint(test_T,"Target");
        csclassPrint(temp_Y,"Predicted");

        for(int i = 0; i < test_T.cols(); i++)
        {
            //max the confusion matrix
            ArrayXXd CNF_MAT = ArrayXXd::Zero(numOfClasses, numOfClasses);

            for(int r = 0; r < test_d; r++)
            {
                if(nnoneof == 0){
                //update the confusion matrix for 2x2
                    if(test_T(r,i) == temp_Y(r,i) && test_T(r,i) <= 0.5)  CNF_MAT(0,0) += 1.0;
                    if(test_T(r,i) != temp_Y(r,i) && test_T(r,i) <= 0.5)  CNF_MAT(1,0) += 1.0;
                    if(test_T(r,i) == temp_Y(r,i) && test_T(r,i) > 0.5)   CNF_MAT(1,1) += 1.0;
                    if(test_T(r,i) != temp_Y(r,i) && test_T(r,i) > 0.5)   CNF_MAT(0,1) += 1.0;
                }
                else
                {
                    CNF_MAT(int(test_T(r,i)),int(temp_Y(r,i))) += 1.0;
                }
                
            }
            

            printf("Confusion Matrix\n");
            std::cout<<CNF_MAT<<std::endl;
        }
        
    }
    else if(python_check)
    {
        std::cout<<RMSE(test_T, temp_Y);
    }

}

//private functions

double NeuralNet::transfer(double x)
{
  return 1.0/(1.0 + std::exp(-4.0 * x));
}

void NeuralNet::feedForward(const Eigen::MatrixXd &input)
{


    //print the hidden layer before the sigmoid


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


double NeuralNet::RMSE(const Eigen::MatrixXd &T, const Eigen::MatrixXd &Y)
{
    MatrixXd error = T-Y;
    error = error.cwiseProduct(error).eval();
    MatrixXd new_error = error.cwiseSqrt().eval();
    return new_error.sum();
}
