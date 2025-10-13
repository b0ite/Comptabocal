#!/usr/bin/env python3
"""
Script de test pour vérifier les améliorations de l'application
"""

import sys
import os
import tkinter as tk

# Ajouter le répertoire stuffs au path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'stuffs'))

try:
    from app import App
    
    print("✅ Import de l'application réussi")
    
    # Test d'initialisation
    root = tk.Tk()
    root.withdraw()  # Cacher la fenêtre de test
    
    try:
        app = App(root)
        print("✅ Initialisation de l'application réussie")
        
        # Vérifier que les attributs importants existent
        required_attrs = ['colors', 'case_fichier1a', 'case_fichier1b', 'case_fichier2', 'case_fichier3a', 'case_fichier3b']
        for attr in required_attrs:
            if hasattr(app, attr):
                print(f"✅ Attribut {attr} présent")
            else:
                print(f"❌ Attribut {attr} manquant")
        
        # Vérifier les nouvelles couleurs
        expected_colors = ['bg', 'section1', 'section2', 'section3', 'text_white', 'border']
        for color in expected_colors:
            if color in app.colors:
                print(f"✅ Couleur {color} définie: {app.colors[color]}")
            else:
                print(f"❌ Couleur {color} manquante")
        
    except Exception as e:
        print(f"❌ Erreur lors de l'initialisation: {e}")
    
    root.destroy()
    
except ImportError as e:
    print(f"❌ Erreur d'import: {e}")

print("\n🔍 Test terminé")
