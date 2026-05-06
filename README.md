## Système de Détection de Collision — Essaim UAV

Projet industriel en **C avancé** simulant un module de sécurité
pour un essaim de 10 000 micro-drones autonomes.

### Objectif
Détecter en temps réel les deux drones les plus proches
pour déclencher une manœuvre d'évitement avant collision.

### Contraintes techniques
- Allocation dynamique (`malloc`) sur un bloc mémoire continu
- Navigation exclusivement par **arithmétique des pointeurs**
- Indexation par crochets `[]` strictement interdite
- Complexité cible : **O(n log n)** au lieu de O(n²)

### Livrables
- Code source C commenté et sécurisé
- Dossier de conception technique
- Audit asymptotique avec preuve formelle de complexité
