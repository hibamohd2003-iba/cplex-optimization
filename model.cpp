#include <ilcplex/ilocplex.h>
#include <fstream>
#include <cmath>

ILOSTLBEGIN

typedef IloArray<IloNumArray>    IloNumArray2;
typedef IloArray<IloNumVarArray> IloNumVarArray2;

int main()
{
    ofstream fout("SolTP-1.txt");
    IloEnv env;

    try {
        // ✅ FIX 1: Open and validate file BEFORE reading
        ifstream fin("data.txt");   // ✅ FIX 2: was "data1.txt" — now matches actual filename
        if (!fin) {
            cerr << "Unable to open data.txt" << endl;
            env.end();
            return 9;
        }

        IloNumArray  Cost(env), Avail(env), DEM(env), REQ(env), PCG(env);
        IloNumArray2 C(env);
        IloNum       PCB, PCGR, PCS;

        // Read order must match data.txt exactly:
        // Cost, Avail, C, DEM, REQ, PCG, PCB, PCGR, PCS
        fin >> Cost >> Avail >> C >> DEM >> REQ >> PCG >> PCB >> PCGR >> PCS;

        IloInt RAW  = Cost.getSize();   // 3 (Oat, Maize, Molasses)
        IloInt COMP = REQ.getSize();    // 3 (Protein, Lipid, Fiber)
        IloInt FOOD = DEM.getSize();    // 2 (Granules, Powder)

        IloModel model(env);

        // ─── Decision variables ───────────────────────────────────────────
        // x[f][r] = kg of raw material r used for food product f
        IloNumVarArray2 x(env, FOOD);
        for (IloInt i = 0; i < FOOD; i++)
            x[i] = IloNumVarArray(env, RAW, 0, IloInfinity, ILOFLOAT);

        // y[f] = total kg of food product f produced
        IloNumVarArray y(env, FOOD, 0, IloInfinity, ILOFLOAT);

        // ─── Objective function ───────────────────────────────────────────
        // T1: raw material cost (all food types)
        IloExpr T1(env);
        for (IloInt i = 0; i < FOOD; i++)
            T1 += IloScalProd(Cost, x[i]);

        // T2: grinding cost (all raw materials except molasses, index 2)
        //     PCG[r] = grinding cost per kg of raw material r (0 for molasses)
        IloExpr T2(env);
        for (IloInt i = 0; i < FOOD; i++)
            T2 += IloScalProd(PCG, x[i]);

        // T3: blending cost (all raw materials, all food types)
        IloExpr T3(env);
        for (IloInt i = 0; i < FOOD; i++)
            for (IloInt j = 0; j < RAW; j++)
                T3 += PCB * x[i][j];

        // T4: granulating cost (food type 0 = granules only)
        IloExpr T4(env);
        for (IloInt j = 0; j < RAW; j++)
            T4 += PCGR * x[0][j];

        // T5: sieving cost (food type 1 = powder only)
        IloExpr T5(env);
        for (IloInt j = 0; j < RAW; j++)
            T5 += PCS * x[1][j];

        model.add(IloMinimize(env, T1 + T2 + T3 + T4 + T5));

        // ✅ FIX 3: End all top-level IloExpr objects after adding to model
        T1.end(); T2.end(); T3.end(); T4.end(); T5.end();

        // ─── Constraint 1: production balance ────────────────────────────
        // sum of raw materials used for food f = amount of food f produced
        for (IloInt i = 0; i < FOOD; i++)
            model.add(IloSum(x[i]) == y[i]);

        // ─── Constraint 2: min nutritional content (Protein & Lipid) ─────
        // For components 0 (Protein) and 1 (Lipid): >= REQ
        for (IloInt i = 0; i < FOOD; i++) {
            for (IloInt j = 0; j < 2; j++) {
                IloExpr DD(env);
                for (IloInt k = 0; k < RAW; k++)
                    DD += C[j][k] * x[i][k];
                model.add(DD >= REQ[j] * y[i]);
                DD.end();  // ✅ FIX 4: Always .end() loop IloExpr
            }
        }

        // ─── Constraint 3: max nutritional content (Fiber) ───────────────
        // For component 2 (Fiber): <= REQ
        for (IloInt i = 0; i < FOOD; i++) {
            IloExpr DD1(env);
            for (IloInt k = 0; k < RAW; k++)
                DD1 += C[2][k] * x[i][k];
            model.add(DD1 <= REQ[2] * y[i]);
            DD1.end();  // ✅ FIX 4
        }

        // ─── Constraint 4: raw material availability ─────────────────────
        // Total use of raw material r across both food types <= AVAIL[r]
        for (IloInt j = 0; j < RAW; j++) {
            IloExpr DD2(env);
            for (IloInt i = 0; i < FOOD; i++)
                DD2 += x[i][j];
            model.add(DD2 <= Avail[j]);
            DD2.end();  // ✅ FIX 4
        }

        // ─── Constraint 5: meet demand ────────────────────────────────────
        for (IloInt i = 0; i < FOOD; i++)
            model.add(y[i] >= DEM[i]);

        // ─── Solve ────────────────────────────────────────────────────────
        IloCplex cplex(model);
        cplex.solve();

        // ─── Output ───────────────────────────────────────────────────────
        env.out() << "Solution Status:  " << cplex.getStatus()   << endl;
        env.out() << "Objective value:  " << cplex.getObjValue() << endl;

        fout << "Solution Status:\t"  << cplex.getStatus()   << endl;
        fout << "Objective value:\t"  << cplex.getObjValue() << endl;

        for (IloInt i = 0; i < FOOD; i++) {
            env.out() << "\n--- Food product " << i
                      << (i == 0 ? " (Granules)" : " (Powder)") << " ---" << endl;
            fout << "\nFood product " << i
                 << (i == 0 ? " (Granules)" : " (Powder)") << endl;

            double total = 0;
            for (IloInt j = 0; j < RAW; j++) {
                double val = cplex.getValue(x[i][j]);
                env.out() << "  x[" << i << "][" << j << "] = " << val << " kg" << endl;
                fout      << "x[" << i << "][" << j << "]\t" << val << endl;
                total += val;
            }
            double yval = cplex.getValue(y[i]);
            env.out() << "  Total produced (y[" << i << "]) = " << yval << " kg" << endl;
            fout      << "y[" << i << "]\t" << yval << endl;
        }

        cplex.exportModel("model.lp");
    }
    catch (IloException& ex) { cerr << "CPLEX error: " << ex << endl; }
    catch (...)               { cerr << "Unknown error"         << endl; }

    env.end();
    return 0;
}
