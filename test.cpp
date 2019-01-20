truct Node
{
    double x;
    double y;
    double dir;
    double corner;
    double state;
}

class Curve
{
  public:
      Curve();

      void addNode(idouble x, double y);
      void insertNode(int index, double x, double y);
      void deleteNode(int index);

      void moveNode(int index, double x, double y);
      void setCorner(int index, double corner);
      void setIncDir(int index, double dir);
      void setOutDir(int index, double dir);
      void setState(int index, int state);
      void setIncState(int index, int state);
      void setOutState(int index, int state);

      void setClosed(int index, bool closed);

      //Transformations
      void translate(double x, double y);
      void rotate(double angle, double pivotX, double pivotY);
      void enlarge(double scaleFactor, double pivotX, double pivotY);
      
      //Optimization
      void S_splineFeasible();
      void S_splineFeasibleSmart();
      void optimize(bool flag_coarse, bool full_refresh);
      int S_Curve(int index, double *rx, double *ry);
      
      int save(fstream &file);
      int load(fstream &file);
      
      //Getters
      int getCount() {return count;};
      bool isClosed() {return closed;};
      double* getNode(int index) {return nodes[index];};
      
  private:
      void pointChanged(int index);

      Node* nodes
      int count, coarseRes;
      bool closed;
      bool *flag_feasible, *flag_optimal;
};



Theres probably a lot of sloppy stuff here, the setting is done by the methods, and the reading is done by getting the struct Node. I agree, it doesnt sound right :P. Basically the user is specifying the Node struct and then the Optimization and Transformation methods change it around. The Curve can be plotted with S_Curve.

In the struct Node, the only really funny one is state, which cannot be set by just setting the variable. the SetIncDir method isnt so important as it just changes the node's corner with some simple calcualtions. Whenever a change is made to a Node, I need (its not maybe anymore) to run the pointChanged method as it does some stuff.

I would like to learn the proper way of doing things and not just some work around. Thanks in advance!
