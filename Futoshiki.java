import javafx.geometry.Pos;
import org.chocosolver.solver.Model;
import org.chocosolver.solver.variables.IntVar;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Futoshiki {
	public static class Position {
		public final int ligne;
		public final int colonne;

		public Position(int ligne, int colonne) {
			this.ligne = ligne;
			this.colonne = colonne;
		}
	}

	public static class Contrainte {
		public final Position petit;
		public final Position grand;

		public Contrainte(int x, int y, char symbole) {
			if (symbole == '^') {
				petit = new Position(x , y);
				grand = new Position(x + 1, y);
			} else if (symbole == 'v') {
				grand = new Position(x , y);
				petit = new Position(x + 1, y);
			} else if (symbole == '<') {
				petit = new Position(x , y);
				grand = new Position(x , y + 1);
			} else if (symbole == '>') {
				petit = new Position(x , y + 1);
				grand = new Position(x , y);
			} else {
				throw new RuntimeException("<" + symbole);
			}
		}

		public void injecterContrainte(Model model, IntVar[][] intVars, int size) {
			model.lexChainLess(new IntVar[] {
					intVars[petit.ligne][petit.colonne],
					intVars[grand.ligne][grand.colonne]
			}).post();
		}
	}

	public void resoudreGrille(int[][] grilleOriginale, List<Contrainte> contraintes) {
		Model model = new Model();

		IntVar[][] intVars = new IntVar[grilleOriginale.length][grilleOriginale.length];

		for (int i = 0 ; i != grilleOriginale.length ; i++) {
			for (int j = 0 ; j != grilleOriginale.length ; j++) {
				if (grilleOriginale[i][j] == 0)
					intVars[i][j] = model.intVar(1, grilleOriginale.length);
				else
					intVars[i][j] = model.intVar(grilleOriginale[i][j]);
			}
		}

		for (int i = 0 ; i != grilleOriginale.length ; i++) {
			model.allDifferent(intVars[i]).post();
		}

		for (int j = 0 ; j != grilleOriginale.length ; j++) {
			IntVar[] colonne = new IntVar[grilleOriginale.length];
			for (int i = 0 ; i != grilleOriginale.length ; i++) {
				colonne[i] = intVars[i][j];
			}

			model.allDifferent(colonne).post();
		}

		for (Contrainte contrainte : contraintes) {
			contrainte.injecterContrainte(model, intVars, grilleOriginale.length);
		}

		if (!model.getSolver().solve()) {
			System.out.println("Pas de solution");
		} else {
			for (int i = 0 ; i != grilleOriginale.length ; i++) {
				for (int j = 0 ; j != grilleOriginale.length ; j++) {
					System.out.print(intVars[i][j].getValue() + " ");
				}

				System.out.println();
			}
		}
	}

	public void resoudreGrille(String fichier) throws IOException {
		int[][] grille = null;
		int taille = 0;
		List<Contrainte> contraintes = new ArrayList<>();

		BufferedReader br = new BufferedReader(new FileReader(fichier));
		String line;
		boolean FOUR = true;
		int numero_ligne = 0;

		while ((line = br.readLine()) != null) {
			if (grille == null) {
				System.out.println(line);
				taille = Integer.parseInt(line);
				System.out.println("Taille = " + taille);
				grille = new int[taille][taille];

				for (int i = 0 ; i != taille ; i++)
					for (int j = 0 ; j != taille ; j++)
						grille[i][j] = 0;
			} else {
				char[] sss = line.toCharArray();
				if (FOUR) {
					for (int i = 0 ; i != sss.length ; i++) {
						if (i % 2 == 0) {
							grille[numero_ligne][i / 2] = sss[i] - '0';
						} else {
							if (sss[i] != ' ') {
								contraintes.add(
										new Futoshiki.Contrainte(numero_ligne, i / 2, sss[i])
								);
							}
						}
					}
				} else {					for (int i = 0 ; i != sss.length ; i++) {

					if (sss[i] != '.') {
						contraintes.add(
								new Futoshiki.Contrainte(numero_ligne, i, sss[i])
						);
					}
				}

					numero_ligne ++;
				}

				FOUR = !FOUR;
			}

			// process the line.
		}
		br.close();


		resoudreGrille(grille, contraintes);
	}


}
