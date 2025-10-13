# Changelog - Générateur de Journaux Comptables

## Version 2.0 - Septembre 2024

### ✨ Nouvelles fonctionnalités
- **Interface modernisée** : Design plus élégant et professionnel
- **Validation des fichiers obligatoires** : Vérification que tous les fichiers requis sont sélectionnés
- **Messages d'erreur améliorés** : Messages plus clairs et informatifs
- **Effets visuels** : Effets de survol sur les boutons
- **Icônes** : Ajout d'icônes pour une meilleure expérience utilisateur

### 🔧 Corrections et améliorations
- **Cohérence terminologique** :
  - "JOURNAL VE" → "JOURNAL VENTE/ENCAISSEMENT" (plus explicite)
  - "Fichier CAISSE-Reglement" → "Fichier CAISSE-Règlement" (orthographe)
  - "Fichier CAISSE-Prlv" → "Fichier CAISSE-Prélèvement" (plus complet)
  - "Fichier BQ-Releve bancaire" → "Fichier BQ-Relevé bancaire" (orthographe)

- **Statut des fichiers** :
  - Le fichier "Plan comptable" n'est plus marqué comme optionnel (maintenant obligatoire)
  - Tous les fichiers sont clairement marqués comme "(obligatoire)"

- **Interface utilisateur** :
  - Palette de couleurs moderne et cohérente
  - Texte blanc sur boutons colorés pour une meilleure lisibilité
  - Espacement et padding améliorés
  - Typographie adaptée au système (SF Pro sur macOS, Segoe UI sur Windows)
  - Séparateurs visuels pour une meilleure organisation

- **Fonctionnalités** :
  - Filtres de fichiers dans la boîte de dialogue (CSV, Excel)
  - Indication visuelle des fichiers sélectionnés (fond vert + icône ✅)
  - Validation des fichiers avant exécution
  - Messages de succès personnalisés par type de journal

### 🎨 Design
- **Couleurs modernes** : Palette inspirée de Tailwind CSS
- **Sections distinctes** : Chaque journal a sa propre couleur thématique
  - Bleu pour Journal Vente/Encaissement
  - Vert pour Journal Caisse  
  - Violet pour Journal Bancaire
- **Cards blanches** : Fond blanc pour les sections de contenu
- **Bordures subtiles** : Bordures grises pour délimiter les zones

### 📱 Compatibilité
- Optimisé pour macOS (polices SF Pro)
- Compatible Windows et Linux
- Interface responsive et scrollable
