import javafx.geometry.Pos;
import org.chocosolver.solver.Model;
import org.chocosolver.solver.variables.IntVar;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Résolution d'instances du jeu de Futoshiki avec le solveur Choco.
 *
 * http://www.choco-solver.org
 */
public class Futoshiki {
	/** Description d'une position */
	public static class Position {
		public final int ligne;
		public final int colonne;

		public Position(int ligne, int colonne) {
			this.ligne = ligne;
			this.colonne = colonne;
		}
	}

	/** Contrainte d'infériorité */
	public static class Contrainte {
		/** Case plus petite */
		public final Position petit;
		/** Case plus grande */
		public final Position grand;

		/**
		 * Construit une contrainte avec la ligne, la colonne et le symbole encodé dans le fichier texte (v, ^,
		 * inférieur ou supérieur)
		 */
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

		/**
		 * Injecte la contrainte dans le modèle
		 * @param model Le modèle choco
		 * @param intVars La liste des variables utilisées
		 * @param size La taille de la grille
		 */
		public void injecterContrainte(Model model, IntVar[][] intVars, int size) {
			model.lexChainLess(new IntVar[] {
					intVars[petit.ligne][petit.colonne],
					intVars[grand.ligne][grand.colonne]
			}).post();
		}
	}

	/**
	 * Résout la grille en affichant une solution
	 * @param grilleOriginale La grille originale, avec 0 si la case est à remplir
	 * @param contraintes La liste des contraintes d'infériorité
	 */
	public void resoudreGrille(int[][] grilleOriginale, List<Contrainte> contraintes) {
		Model model = new Model();

		// Liste des variables : une variable par case
		IntVar[][] intVars = new IntVar[grilleOriginale.length][grilleOriginale.length];

		for (int i = 0 ; i != grilleOriginale.length ; i++) {
			for (int j = 0 ; j != grilleOriginale.length ; j++) {
				if (grilleOriginale[i][j] == 0)
					// Domaine de 1:taille
					intVars[i][j] = model.intVar(1, grilleOriginale.length);
				else
					// Domaine égal à la valeur
					intVars[i][j] = model.intVar(grilleOriginale[i][j]);
			}
		}

		// Toutes les lignes et colonnes ont des cases différentes
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

		// Ajout des contraintes d'infériorité
		for (Contrainte contrainte : contraintes) {
			contrainte.injecterContrainte(model, intVars, grilleOriginale.length);
		}

		// Résolution et affichage
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

	/**
	 * Résout la grille contenue dans le fichier donné
	 * @param fichier Le fichier
	 * @throws IOException
	 */
	public void resoudreGrille(String fichier) throws IOException {
		// Lecture du fichier
		int[][] grille = null;
		int taille = 0;
		List<Contrainte> contraintes = new ArrayList<>();

		BufferedReader br = new BufferedReader(new FileReader(fichier));
		String line;
		boolean ligneDeValeurs = true;
		int numeroLigne = 0;

		while ((line = br.readLine()) != null) {
			if (grille == null) {
				// Première ligne : taille de la grille
				taille = Integer.parseInt(line);
				grille = new int[taille][taille];

				for (int i = 0 ; i != taille ; i++)
					for (int j = 0 ; j != taille ; j++)
						grille[i][j] = 0;
			} else {
				char[] caracteres = line.toCharArray();
				// TODO : supporter les grilles de tailles supérieures à 9
				if (ligneDeValeurs) {
					for (int i = 0 ; i != caracteres.length ; i++) {
						if (i % 2 == 0) {
							grille[numeroLigne][i / 2] = caracteres[i] - '0';
						} else {
							if (caracteres[i] != ' ') {
								contraintes.add(
										new Futoshiki.Contrainte(numeroLigne, i / 2, caracteres[i])
								);
							}
						}
					}
				} else {
					for (int i = 0 ; i != caracteres.length ; i++) {
						if (caracteres[i] != '.') {
							contraintes.add(
									new Futoshiki.Contrainte(numeroLigne, i, caracteres[i])
							);
						}
					}

					numeroLigne++;
				}

				ligneDeValeurs = !ligneDeValeurs;
			}
		}
		br.close();

		// Résolution
		resoudreGrille(grille, contraintes);
	}


}
