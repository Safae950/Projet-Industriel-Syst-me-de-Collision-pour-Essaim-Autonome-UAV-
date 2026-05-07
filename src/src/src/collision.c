/**
 * ============================================================
 * SYSTÈME DE DÉTECTION DE COLLISION — ESSAIM UAV
 * École des Sciences de l'Information | Pr. Tarik HOUICHIME
 * ============================================================
 *
 * Objectif : Identifier en O(n log n) les deux drones les plus
 *            proches parmi 10 000, SANS jamais utiliser essaim[i].
 *
 * Contraintes respectées :
 *   - Arithmétique pure des pointeurs  (*(ptr + offset))
 *   - Interdiction totale de l'opérateur []
 *   - Allocation dynamique unique (malloc continu)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

/* ─────────────────────────────────────────────
   1. STRUCTURE DE DONNÉES
   ───────────────────────────────────────────── */

typedef struct {
    int   id;
    float x;
    float y;
    float z;
} Drone;

/* ─────────────────────────────────────────────
   2. DISTANCE EUCLIDIENNE AU CARRÉ
   (évite sqrt inutile pour les comparaisons)
   ───────────────────────────────────────────── */

static inline float dist2(const Drone *a, const Drone *b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return dx*dx + dy*dy + dz*dz;
}

/* ─────────────────────────────────────────────
   3. TRI RAPIDE (QuickSort) SUR L'AXE X
   — SANS crochets, arithmétique de pointeurs —
   ───────────────────────────────────────────── */

/**
 * Échange deux Drones en mémoire via leurs pointeurs.
 */
static void swap_drones(Drone *a, Drone *b) {
    Drone tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * Partition de Lomuto — pivot = dernier élément.
 * Utilise exclusivement l'arithmétique *(base + offset).
 */
static int partition(Drone *base, int lo, int hi) {
    float pivot_x = (base + hi)->x;
    int   i       = lo - 1;
    int   j;

    for (j = lo; j < hi; j++) {
        if ((base + j)->x <= pivot_x) {
            i++;
            swap_drones(base + i, base + j);
        }
    }
    swap_drones(base + i + 1, base + hi);
    return i + 1;
}

/**
 * QuickSort récursif sur l'axe X.
 * Complexité moyenne : O(n log n)
 */
static void quicksort_x(Drone *base, int lo, int hi) {
    if (lo >= hi) return;
    int p = partition(base, lo, hi);
    quicksort_x(base, lo,   p - 1);
    quicksort_x(base, p + 1, hi  );
}

/* ─────────────────────────────────────────────
   4. RECHERCHE DE LA PAIRE LA PLUS PROCHE
      Algorithme de balayage (Sweep Line)
      Complexité : O(n log n) global
   ───────────────────────────────────────────── */

/**
 * Après tri sur X, on maintient une fenêtre glissante :
 * pour chaque drone i, on ne compare qu'avec les drones j
 * tels que |xj - xi| < min_dist_courante.
 * Cela garantit O(n log n) en pratique.
 *
 * @param essaim   Tableau de Drones trié sur X
 * @param n        Nombre de drones
 * @param id_a     [out] ID du premier  drone de la paire
 * @param id_b     [out] ID du deuxième drone de la paire
 * @return         Distance euclidienne minimale
 */
float trouver_paire_proche(Drone *essaim, int n, int *id_a, int *id_b) {
    float min_dist2 = FLT_MAX;
    int   i, j;

    for (i = 0; i < n - 1; i++) {
        Drone *di = essaim + i;           /* arithmétique de pointeur */

        for (j = i + 1; j < n; j++) {
            Drone *dj = essaim + j;       /* arithmétique de pointeur */

            /* Élagage sur X : si trop loin, arrêt de la fenêtre */
            float dx = dj->x - di->x;
            if (dx * dx >= min_dist2) break;

            float d2 = dist2(di, dj);
            if (d2 < min_dist2) {
                min_dist2 = d2;
                *id_a = di->id;
                *id_b = dj->id;
            }
        }
    }
    return sqrtf(min_dist2);
}

/* ─────────────────────────────────────────────
   5. GÉNÉRATION ALÉATOIRE DE L'ESSAIM
   ───────────────────────────────────────────── */

/**
 * Remplit le tableau avec des positions aléatoires dans
 * un espace 3D de 1 000 m × 1 000 m × 500 m.
 * Uniquement arithmétique de pointeurs.
 */
void generer_essaim(Drone *essaim, int n) {
    int i;
    for (i = 0; i < n; i++) {
        Drone *d = essaim + i;
        d->id = i + 1;
        d->x  = ((float)rand() / RAND_MAX) * 1000.0f;
        d->y  = ((float)rand() / RAND_MAX) * 1000.0f;
        d->z  = ((float)rand() / RAND_MAX) *  500.0f;
    }
}

/* ─────────────────────────────────────────────
   6. PROGRAMME PRINCIPAL
   ───────────────────────────────────────────── */

int main(void) {
    const int N = 10000;
    int       id_a = -1, id_b = -1;
    clock_t   t0, t1;

    printf("=== Système de Détection de Collision UAV ===\n");
    printf("Nombre de drones : %d\n\n", N);

    /* ── Allocation dynamique unique (un seul bloc contigu) ── */
    Drone *essaim = (Drone *)malloc(N * sizeof(Drone));
    if (!essaim) {
        fprintf(stderr, "[ERREUR FATALE] malloc échoué — mémoire insuffisante\n");
        return EXIT_FAILURE;
    }

    /* ── Génération de l'essaim ── */
    srand((unsigned int)time(NULL));
    generer_essaim(essaim, N);

    /* ── Phase 1 : Tri sur l'axe X  O(n log n) ── */
    t0 = clock();
    quicksort_x(essaim, 0, N - 1);

    /* ── Phase 2 : Balayage fenêtré  O(n log n) ── */
    float dist = trouver_paire_proche(essaim, N, &id_a, &id_b);
    t1 = clock();

    /* ── Résultats ── */
    double elapsed_ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;

    printf("[ALERTE COLLISION]\n");
    printf("  Drone #%-5d  →  position (%.2f, %.2f, %.2f)\n",
           id_a,
           (essaim + id_a - 1)->x,
           (essaim + id_a - 1)->y,
           (essaim + id_a - 1)->z);
    printf("  Drone #%-5d  →  position (%.2f, %.2f, %.2f)\n",
           id_b,
           (essaim + id_b - 1)->x,
           (essaim + id_b - 1)->y,
           (essaim + id_b - 1)->z);
    printf("  Distance minimale : %.6f m\n\n", dist);
    printf("[PERFORMANCE]\n");
    printf("  Temps d'exécution  : %.3f ms\n", elapsed_ms);
    printf("  Complexité globale : O(n log n)\n");

    /* ── Libération mémoire ── */
    free(essaim);
    essaim = NULL;

    return EXIT_SUCCESS;
}
