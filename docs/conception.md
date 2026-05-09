Dossier de Conception Technique : 
Introduction : 
Dans ce dossier, je vais expliquer les choix que j'ai fait pour concevoir ce système de 
détection de collision. Mon objectif principal était de respecter les contraintes imposées (pas 
de crochets, arithmétique de pointeurs pure) tout en obtenant un programme rapide et sûr. 
1. Comment j'ai organisé la mémoire ? 
La première chose que j'ai décidée, c'est d'allouer tous les drones d'un seul coup dans un bloc 
mémoire unique : 
Drone *essaim = (Drone *)malloc(N * sizeof(Drone)); 
Pourquoi un seul malloc ? Parce que si j'avais alloué chaque drone séparément, ils auraient 
été éparpillés en mémoire et je n'aurais pas pu naviguer entre eux par arithmétique de 
pointeurs. Avec un bloc contigu, je sais exactement où se trouve le drone numéro i : il suffit 
de déplacer le pointeur de base de i positions. 
Concrètement, le bloc fait 200 000 octets en mémoire (10 000 drones × 20 octets chacun : 4 
pour id, 4 pour x, 4 pour y, 4 pour z). 
2. La navigation sans crochets : 
C'est la contrainte la plus particulière du projet. Partout dans le code, au lieu d'écrire 
essaim[i].x, j'écris (essaim + i)->x. Ces deux écritures produisent exactement le même 
code machine, mais la deuxième respecte l'interdiction du compilateur de sécurité. 
Dans la fonction generer_essaim, par exemple, j'utilise un pointeur local d que je positionne 
sur chaque drone : 
Drone *d = essaim + i; 
d->id = i + 1; 
d->x  = ((float)rand() / RAND_MAX) * 1000.0f; 
À chaque tour de boucle, d pointe sur le drone i. J'accède ensuite à ses champs avec -> sans 
jamais utiliser de crochets. 
3. Pourquoi j'ai choisi de trier sur l'axe X 
L'idée de trier les drones avant de chercher la paire la plus proche vient d'une observation 
simple : si deux drones sont très proches en distance 3D, ils sont forcément proches aussi sur 
l'axe X. Autrement dit : 
Si la distance réelle entre A et B est d, alors l'écart |x_B - x_A| est toujours inférieur ou égal 
à d. 
Donc, une fois les drones triés par X, je peux arrêter de comparer le drone i avec le drone j 
dès que leur écart en X dépasse la meilleure distance trouvée jusqu'ici. Tous les drones 
suivants seraient encore plus loin, inutile d'aller plus loin. 
C'est ce que fait ce break dans la boucle : 
float dx = dj->x - di->x; 
if (dx * dx >= min_dist2) break; 
4. Le tri : QuickSort avec partition de Lomuto : 
Pour trier, j'ai implémenté un QuickSort récursif. J'ai choisi la partition de Lomuto car elle est 
simple à coder sans indices tableau. Le pivot est toujours le dernier élément du segment : 
float pivot_x = (base + hi)->x; 
L'échange physique de deux drones se fait dans swap_drones. Cette fonction copie la 
structure entière (20 octets) dans une variable temporaire avant de permuter : 
Drone tmp = *a; 
*a = *b; 
*b = tmp; 
Ce point est important : ce sont les structures complètes qui se déplacent dans le bloc 
mémoire, pas des pointeurs. Après le tri, le bloc reste contigu et cohérent. 
5. Le calcul de distance : pourquoi je n'utilise pas sqrt dans la boucle : 
Dans la fonction dist2, je retourne la distance au carré : 
return dx*dx + dy*dy + dz*dz; 
La raison est simple : comparer d1² < d2² donne le même résultat que comparer d1 < d2 
quand les deux valeurs sont positives. Donc pendant tout le balayage, je travaille avec 
min_dist2 (la meilleure distance au carré). Je n'appelle sqrtf qu'une seule fois, tout à la fin, 
pour afficher le résultat : 
return sqrtf(min_dist2); 
Sur un processeur embarqué, sqrtf est une opération relativement coûteuse. Éviter de 
l'appeler dans une boucle qui tourne des millions de fois représente un gain de temps réel non 
négligeable. 
6. Un détail important sur les identifiants 
Lors de la génération, j'initialise les identifiants à i + 1 (base 1) : 
d->id = i + 1; 
Après le tri, les drones changent de position dans le tableau, mais leur champ id se déplace 
avec eux (puisque swap_drones copie la structure entière). Donc dans main, pour retrouver la 
position du drone dont l'id est id_a, j'écris : 
(essaim + id_a - 1)->x 
Le -1 vient du fait que l'id commence à 1 mais le tableau commence à 0. C'est un point subtil 
que j'ai dû faire attention à ne pas oublier lors de l'affichage. 
7. Sécurité et gestion mémoire : 
J'ai ajouté une vérification après malloc pour éviter un crash silencieux si la mémoire est 
insuffisante : 
if (!essaim) { 
fprintf(stderr, "[ERREUR FATALE] malloc échoué\n"); 
return EXIT_FAILURE; 
} 
Et à la fin du programme, je libère le bloc et je mets le pointeur à NULL pour éviter tout accès 
à une zone mémoire libérée : 
free(essaim); 
essaim = NULL; 
Conclusion : 
Les choix que j'ai faits «  bloc contigu, tri QuickSort sur X, fenêtre glissante, distance au 
carré»forment un ensemble cohérent qui répond aux deux exigences du cahier des charges : 
respecter l'arithmétique pure des pointeurs, et dépasser les limites de l'approche naïve O(n²).  
