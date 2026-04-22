# Cost Optimization Using CPLEX

A linear programming solution for raw material blending to minimize production cost, implemented using IBM ILOG CPLEX Optimization Studio.

---

## Problem Description

This project solves two related cost optimization problems in the context of industrial production planning.

### Problem 1 — Steel Blending
A steel company needs to produce 500 tonnes of steel for shipbuilding. The steel must satisfy minimum and maximum grade requirements for three chemical elements: Carbon (C), Copper (Cu), and Manganese (Mn). Seven raw materials (iron alloys, copper alloys, aluminum alloys) are available, each with a known cost, availability, and chemical composition. The objective is to find the optimal mix of raw materials that meets all grade requirements at minimum cost.

### Problem 2 — Animal Food Production (CowFood)
A food company produces two animal food products: granules and powder. Three raw materials are used: oat, maize, and molasses. Each product must satisfy nutritional requirements for protein, lipids, and fiber. Production involves multiple steps (grinding, blending, granulating, sieving), each with an associated cost. The objective is to determine the optimal quantities of each raw material for each product to minimize total production cost while meeting daily demand.

---

## Model Formulation

The linear programming model uses the following structure:

- **Decision variables** — quantity of each raw material used for each product
- **Objective function** — minimize total cost (raw material + processing costs)
- **Constraints:**
  - Nutritional/grade composition limits (min and max)
  - Raw material availability limits
  - Production demand requirements
  - Non-negativity conditions

---

## Project Structure

```
├── model.cpp       # CPLEX C++ implementation
├── data.txt        # Input data (costs, availabilities, compositions, demand)
├── report.pdf      # Full problem description and model formulation
└── README.md       # This file
```

---

## Input Data Format (`data.txt`)

The data file is read in this exact order using CPLEX `IloNumArray` notation (space-separated, no commas):

```
[cost_per_raw_material]
[availability_per_raw_material]
[[composition_matrix_rows]]
[demand_per_product]
[nutritional_requirements]
[grinding_cost_per_raw_material]
blending_cost
granulating_cost
sieving_cost
```

**Example (`data.txt`):**
```
[0.13 0.17 0.12]
[11900 23500 750]
[[13.6 4.1 5][7.1 2.4 0.3][7 3.7 25]]
[9000 12000]
[9.5 2 6]
[0.25 0.25 0]
0.05
0.42
0.17
```

---

## Requirements

- IBM ILOG CPLEX Optimization Studio (version 12.x or later)
- C++ compiler (MSVC via Visual Studio 2019/2022 recommended on Windows)
- Windows 10/11 (64-bit)

---

## How to Build and Run

### Visual Studio (Windows)

1. Open Visual Studio and create a new **Empty C++ Project**
2. Add `model.cpp` to the project sources
3. Configure CPLEX include and library paths:
   - Go to **Project → Properties → C/C++ → General → Additional Include Directories**
   - Add: `C:\Program Files\IBM\ILOG\CPLEX_StudioXXX\cplex\include`
   - Go to **Linker → General → Additional Library Directories**
   - Add: `C:\Program Files\IBM\ILOG\CPLEX_StudioXXX\cplex\lib\x64_windows_msvc14\stat_mda`
   - Go to **Linker → Input → Additional Dependencies**
   - Add: `cplex2010.lib` (adjust version number to match your installation)
4. Set the **Working Directory**:
   - Go to **Project → Properties → Debugging → Working Directory**
   - Set to: `$(ProjectDir)`
5. Place `data.txt` in the project folder (same folder as the `.vcxproj` file)
6. Build the solution (**Ctrl+Shift+B**) and run (**Ctrl+F5**)

---

## Expected Output

```
Solution Status:  Optimal
Objective value:  <minimum cost>

--- Food product 0 (Granules) ---
  x[0][0] = <oat used> kg
  x[0][1] = <maize used> kg
  x[0][2] = <molasses used> kg
  Total produced (y[0]) = 9000 kg

--- Food product 1 (Powder) ---
  x[1][0] = <oat used> kg
  x[1][1] = <maize used> kg
  x[1][2] = <molasses used> kg
  Total produced (y[1]) = 12000 kg
```

Results are also saved to `SolTP-1.txt` and the LP model is exported to `model.lp`.

---

## Key Fixes Applied to Original Code

| Issue | Fix |
|---|---|
| File check after reading | Moved `if (!fin)` before `fin >>` |
| Wrong filename `data1.txt` | Changed to `data.txt` |
| Commas in `data.txt` | Replaced with spaces (CPLEX requirement) |
| `IloExpr` memory leaks | Added `.end()` after every expression |
| Global loop indices | Made all loop variables local |
| Removed `conio.h` | Unused Windows-only header removed |

---

## References

- IBM ILOG CPLEX Optimization Studio Documentation
- Course report: Problem 6.1 (Steel Blending) and Problem 6.2 (CowFood Production)
