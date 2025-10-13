import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import os
import subprocess
import platform
import webbrowser

# Infobulles légères façon macOS
class Tooltip:
    def __init__(self, widget, text):
        self.widget = widget
        self.text = text
        self.tip = None
        widget.bind("<Enter>", self.show)
        widget.bind("<Leave>", self.hide)

    def show(self, event=None):
        if self.tip is not None:
            return
        x = self.widget.winfo_rootx() + self.widget.winfo_width() // 2
        y = self.widget.winfo_rooty() + self.widget.winfo_height() + 10
        self.tip = tk.Toplevel(self.widget)
        self.tip.wm_overrideredirect(True)
        self.tip.configure(bg="#000000", padx=8, pady=6)
        label = tk.Label(self.tip, text=self.text, bg="#000000", fg="#ffffff", font=("SF Pro Text", 10) if platform.system()=="Darwin" else ("Segoe UI", 9))
        label.pack()
        self.tip.wm_geometry(f"+{x}+{y}")

    def hide(self, event=None):
        if self.tip is not None:
            self.tip.destroy()
            self.tip = None

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("Générateur de Journaux Comptables")
        self.root.geometry("1200x900")
        self.root.minsize(900, 700)
        self.root.configure(bg="#f5f7fa")
        self.fichiers = []
        self.last_dir = os.path.join(os.path.expanduser("~"), "Desktop")
        
        # Détection du thème sombre sur macOS
        self.is_dark = self._is_macos_dark_mode() if platform.system() == "Darwin" else False
        
        # Définition des couleurs pour un thème cohérent et moderne
        self.colors = {
            # Fond général type macOS
            "bg": "#f5f5f7",
            "section1": "#e0f2fe",  # Bleu clair pour Journal Vente
            "section2": "#f0fdf4",  # Vert clair pour Journal Caisse  
            "section3": "#fef7ff",  # Violet clair pour Journal Bancaire
            # Boutons
            "btn_add": "#047857",   # Vert foncé pour bon contraste
            "btn_del": "#dc2626",   # Rouge foncé pour bon contraste
            "btn_exec1": "#1e40af", # Bleu foncé pour bon contraste
            "btn_exec2": "#065f46", # Vert très foncé pour bon contraste
            "btn_exec3": "#581c87", # Violet foncé pour bon contraste
            # Texte
            "text": "#1d1d1f",
            "text_white": "#ffffff",
            "text_dark": "#374151", # Texte foncé pour boutons clairs
            "muted_text": "#6e6e73", # Texte secondaire type Apple
            # Couleurs de sections
            "section_text1": "#0369a1", # Bleu foncé
            "section_text2": "#047857", # Vert foncé
            "section_text3": "#6b21a8", # Violet foncé
            # Bordures et états
            "border": "#d2d2d7",
            "hover": "#f3f4f6",
            # Accent macOS
            "accent": "#0a84ff"
        }

        # Polices cohérentes façon Apple
        if platform.system() == "Darwin":
            self.font_title = ("SF Pro Display", 24, "bold")
            self.font_section = ("SF Pro Display", 16, "bold")
            self.font_button = ("SF Pro Display", 13, "bold")
            self.font_text = ("SF Pro Text", 12)
        else:
            self.font_title = ("Segoe UI", 22, "bold")
            self.font_section = ("Segoe UI", 15, "bold")
            self.font_button = ("Segoe UI", 12, "bold")
            self.font_text = ("Segoe UI", 11)

        # Registre des boutons d'exécution et labels requis
        self.exec_buttons = []

        # Rendre l'interface responsive
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        
        # Ajuster la palette si macOS en mode sombre
        if platform.system() == "Darwin" and self._is_macos_dark_mode():
            self.colors.update({
                "bg": "#1c1c1e",
                "text": "#f5f5f7",
                "text_dark": "#e5e7eb",
                "muted_text": "#a1a1aa",
                "border": "#3a3a3c",
                "hover": "#2c2c2e",
            })

        # Déterminer si le thème sombre macOS est actif
        self.is_dark = platform.system() == "Darwin" and self._is_macos_dark_mode()

        
        self.build_ui()

    def _is_macos_dark_mode(self):
        """Détecte si macOS est en mode sombre"""
        try:
            import subprocess
            result = subprocess.run(['defaults', 'read', '-g', 'AppleInterfaceStyle'], 
                                  capture_output=True, text=True)
            return result.stdout.strip() == 'Dark'
        except:
            return False

    def darken_color(self, color):
        """Assombrit une couleur pour l'effet hover"""
        color_map = {
            self.colors["btn_exec1"]: "#1e3a8a",  # Bleu encore plus foncé
            self.colors["btn_exec2"]: "#064e3b",  # Vert encore plus foncé
            self.colors["btn_exec3"]: "#4c1d95",  # Violet encore plus foncé
            self.colors["btn_add"]: "#064e3b",    # Vert encore plus foncé
            self.colors["btn_del"]: "#b91c1c"     # Rouge encore plus foncé
        }
        return color_map.get(color, color)

    def _is_macos_dark_mode(self):
        if platform.system() != "Darwin":
            return False
        try:
            result = subprocess.run([
                "defaults", "read", "-g", "AppleInterfaceStyle"
            ], capture_output=True, text=True)
            return result.returncode == 0 and "Dark" in result.stdout
        except Exception:
            return False

    def _build_menubar(self):
        menubar = tk.Menu(self.root)
        menu_fichier = tk.Menu(menubar, tearoff=0)
        menu_aide = tk.Menu(menubar, tearoff=0)
        # Fichier
        menu_fichier.add_command(label="Quitter", accelerator="⌘Q" if platform.system()=="Darwin" else "Ctrl+Q", command=self.root.quit)
        menubar.add_cascade(label="Fichier", menu=menu_fichier)
        # Aide
        menu_aide.add_command(label="À propos", command=self._show_about)
        menu_aide.add_command(label="Site web", command=lambda: webbrowser.open("https://example.com"))
        menubar.add_cascade(label="Aide", menu=menu_aide)
        self.root.config(menu=menubar)
        # Raccourcis clavier
        if platform.system()=="Darwin":
            self.root.bind_all("<Command-q>", lambda e: self.root.quit())
        else:
            self.root.bind_all("<Control-q>", lambda e: self.root.quit())

    def _show_about(self):
        messagebox.showinfo(
            "À propos",
            "Générateur de Journaux Comptables\n\nDesign soigné, inspiré par macOS.\n© 2024"
        )

    def build_ui(self):
        # Harmoniser le fond racine avec la palette
        self.root.configure(bg=self.colors["bg"])  # unifier le fond
        
        # Création d'un canvas avec scrollbar pour permettre le défilement
        self.outer_frame = tk.Frame(self.root, bg=self.colors["bg"])
        self.outer_frame.pack(fill=tk.BOTH, expand=True)

        # Configurer les proportions
        self.outer_frame.columnconfigure(0, weight=1)
        self.outer_frame.rowconfigure(0, weight=1)

        # Créer le canvas avec scrollbar et configuration pour défilement fluide
        self.canvas = tk.Canvas(self.outer_frame, bg=self.colors["bg"], highlightthickness=0)
        self.scrollbar = ttk.Scrollbar(self.outer_frame, orient="vertical", command=self.canvas.yview)
        self.scrollable_frame = tk.Frame(self.canvas, bg=self.colors["bg"])
        
        # Configuration pour un défilement plus fluide
        self.canvas.configure(scrollregion=(0, 0, 0, 2000))  # Grande région de scroll
        
        # Configurer le scrollable_frame pour qu'il se redimensionne avec le canvas
        self.scrollable_frame.bind(
            "<Configure>",
            lambda e: self.canvas.configure(
                scrollregion=self.canvas.bbox("all")
            )
        )
        
        # Créer une fenêtre dans le canvas qui contient le frame scrollable (conserver l'id pour resize)
        self.scrollable_window = self.canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw", width=self.canvas.winfo_width())
        self.canvas.configure(yscrollcommand=self.scrollbar.set)
        
        # Configurer le remplissage et l'expansion
        self.canvas.pack(side="left", fill="both", expand=True)
        self.scrollbar.pack(side="right", fill="y")
        
        # Configurer le redimensionnement du canvas
        self.canvas.bind("<Configure>", self.on_canvas_configure)
        
        # Titre principal avec style amélioré
        titre_frame = tk.Frame(self.scrollable_frame, bg=self.colors["bg"], pady=20)
        titre_frame.pack(fill=tk.X)
        
        titre = tk.Label(titre_frame, text="🧾 Générateur de Journaux Comptables", font=self.font_title, bg=self.colors["bg"], fg=self.colors["text"]) 
        titre.pack()
        
        subtitle = tk.Label(titre_frame, text="Générez automatiquement vos journaux comptables à partir de vos fichiers", font=(self.font_text[0], self.font_text[1]-1) if platform.system() != "Darwin" else ("SF Pro Text", 13), bg=self.colors["bg"], fg=self.colors["muted_text"]) 
        subtitle.pack(pady=(5, 15))

        # Conteneur principal avec marges
        main_container = tk.Frame(self.scrollable_frame, bg=self.colors["bg"], padx=40, pady=20)
        main_container.pack(fill=tk.BOTH, expand=True)

        # Section 1: Journal Vente/Encaissement (VE)
        self.create_section(
            main_container, 
            "JOURNAL VENTE/ENCAISSEMENT", 
            "1",
            self.colors["section1"], 
            self.colors["section_text1"],
            [
                ("📄 Fichier CAISSE-CA (obligatoire)", self.ajouter_fichier1a),
                ("💳 Fichier CAISSE-Règlement (obligatoire)", self.ajouter_fichier1b)
            ],
            "Générer Journal Vente",
            self.colors["btn_exec1"],
            self.executer_script1
        )
        
        # Section 2: Journal Caisse
        self.create_section(
            main_container, 
            "JOURNAL CAISSE", 
            "2",
            self.colors["section2"], 
            self.colors["section_text2"],
            [
                ("🏦 Fichier CAISSE-Prélèvement (obligatoire)", self.ajouter_fichier2)
            ],
            "Générer Journal Caisse",
            self.colors["btn_exec2"],
            self.executer_script2
        )
        
        # Section 3: Journal Bancaire
        self.create_section(
            main_container, 
            "JOURNAL BANCAIRE", 
            "3",
            self.colors["section3"], 
            self.colors["section_text3"],
            [
                ("🏛️ Fichier BQ-Relevé bancaire (obligatoire)", self.ajouter_fichier3a),
                ("📋 Fichier Plan comptable (obligatoire)", self.ajouter_fichier3b)
            ],
            "Générer Journal Bancaire",
            self.colors["btn_exec3"],
            self.executer_script3
        )
        
        # Pied de page
        footer_frame = tk.Frame(self.scrollable_frame, bg=self.colors["bg"], pady=20)
        footer_frame.pack(fill=tk.X)
        
        separator = tk.Frame(footer_frame, bg=self.colors["border"], height=1)
        separator.pack(fill=tk.X, padx=40, pady=(0, 15))
        
        footer = tk.Label(footer_frame, text="© 2024 - Outil de génération de journaux comptables", 
                        font=("SF Pro Text", 10) if platform.system() == "Darwin" else ("Segoe UI", 9), 
                        bg=self.colors["bg"], fg="#9ca3af")
        footer.pack()
        
        # Remplacer la liaison de défilement par une version plus robuste et fluide
        if platform.system() == "Windows":
            self.canvas.bind_all("<MouseWheel>", self._on_mousewheel_windows)
        elif platform.system() == "Darwin":  # macOS
            self.canvas.bind_all("<MouseWheel>", self._on_mousewheel_macos)
            # Ajouter support pour le trackpad sur macOS
            self.canvas.bind_all("<Button-4>", self._on_mousewheel_macos_trackpad)
            self.canvas.bind_all("<Button-5>", self._on_mousewheel_macos_trackpad)
        else:  # Linux et autres
            self.canvas.bind_all("<Button-4>", self._on_mousewheel_linux)
            self.canvas.bind_all("<Button-5>", self._on_mousewheel_linux)

        self._build_menubar()

    def on_canvas_configure(self, event):
        # Met à jour la taille de la fenêtre du scrollable_frame quand le canvas est redimensionné
        try:
            self.canvas.itemconfig(self.scrollable_window, width=event.width)
        except Exception:
            # fallback silencieux si non dispo
            pass

    def _on_mousewheel_windows(self, event):
        # Windows - défilement fluide
        self.canvas.yview_scroll(int(-1*(event.delta/120)), "units")
    
    def _on_mousewheel_macos(self, event):
        # macOS - défilement ultra fluide avec sensibilité ajustée
        # Obtenir la position actuelle
        top, bottom = self.canvas.yview()
        
        # Calculer le déplacement avec sensibilité plus fine
        scroll_amount = event.delta * 0.005  # Très petits incréments pour fluidité maximale
        
        # Nouvelle position
        new_top = max(0, min(1, top - scroll_amount))
        
        # Appliquer le défilement fluide
        self.canvas.yview_moveto(new_top)
        
    def _on_mousewheel_macos_trackpad(self, event):
        # macOS trackpad - défilement extra fluide avec petits incréments
        top, bottom = self.canvas.yview()
        
        if event.num == 4:
            new_top = max(0, top - 0.02)  # Remonte par petits pas
        elif event.num == 5:
            new_top = min(1, top + 0.02)  # Descend par petits pas
        else:
            return
            
        self.canvas.yview_moveto(new_top)
    
    def _on_mousewheel_linux(self, event):
        # Linux - défilement par petites unités
        if event.num == 4:
            self.canvas.yview_scroll(-1, "units")
        elif event.num == 5:
            self.canvas.yview_scroll(1, "units")

    def create_section(self, parent, title, number, bg_color, text_color, file_items, btn_text, btn_color, btn_command):
        # Cadre de section avec design moderne
        section_frame = tk.Frame(parent, bg=self.colors["bg"], pady=15)
        section_frame.pack(fill=tk.BOTH, expand=True)
        
        # En-tête avec style moderne
        header_bg = tk.Frame(section_frame, bg=bg_color, padx=20, pady=15, relief=tk.FLAT)
        header_bg.pack(fill=tk.X, padx=0, pady=(0, 1))
        
        section_title = tk.Label(header_bg, text=f"{number}. {title}", font=self.font_section, bg=bg_color, fg=text_color)
        section_title.pack(anchor="w")
        
        # Cadre principal avec design moderne
        content_frame = tk.Frame(section_frame, bg="#ffffff", bd=0, relief=tk.FLAT, padx=20, pady=20)
        content_frame.pack(fill=tk.BOTH, expand=True, padx=0, pady=(0, 20))
        
        # Ajouter une bordure subtile
        content_frame.configure(highlightbackground=self.colors["border"], highlightthickness=1)
        
        # Zone de fichiers
        files_frame = tk.Frame(content_frame, bg="#ffffff")
        files_frame.pack(fill=tk.BOTH, expand=True)
        
        # Créer les cases pour chaque fichier
        file_labels = []
        for file_title, callback in file_items:
            file_label = self.creer_case_fichier_moderne(file_title, callback, files_frame)
            file_labels.append(file_label)
            
        # Attribuer les labels aux variables d'instance
        if title == "JOURNAL VENTE/ENCAISSEMENT":
            self.case_fichier1a, self.case_fichier1b = file_labels if len(file_labels) > 1 else [file_labels[0], None]
        elif title == "JOURNAL CAISSE":
            self.case_fichier2 = file_labels[0]
        elif title == "JOURNAL BANCAIRE":
            self.case_fichier3a = file_labels[0]
            self.case_fichier3b = file_labels[1] if len(file_labels) > 1 else None
        
        # Bouton d'exécution avec style moderne
        exec_btn = tk.Button(content_frame, 
                           text=f"▶️  {btn_text}", 
                           command=btn_command,
                           bg=btn_color, 
                           fg=self.colors["text"], 
                           font=self.font_button, 
                           relief=tk.FLAT, 
                           padx=20, 
                           pady=12,
                           cursor="hand2",
                           activebackground=self.darken_color(btn_color),
                           activeforeground=self.colors["text"],
                           bd=0,
                           takefocus=True,
                           highlightthickness=2,
                           highlightbackground=self.colors["border"],
                           highlightcolor=self.colors["accent"]) 
        exec_btn.pack(pady=(20, 0), fill=tk.X)
        # Active avec Entrée
        exec_btn.bind("<Return>", lambda e: exec_btn.invoke())
        
        # Enregistrer et initialiser l'état du bouton selon les fichiers requis
        self._register_exec_button(exec_btn, file_labels, btn_color)

        # Infobulle pour le bouton Générer
        self._add_tooltip(exec_btn, "Lancer la génération du journal")

        # Ajouter des effets de survol
        def on_enter(e):
            exec_btn.configure(bg=self.darken_color(btn_color))
        def on_leave(e):
            exec_btn.configure(bg=btn_color)
        
        exec_btn.bind("<Enter>", on_enter)
        exec_btn.bind("<Leave>", on_leave)

    def _register_exec_button(self, btn, required_labels, base_color):
        btn.required_labels = required_labels
        btn.base_color = base_color
        self.exec_buttons.append(btn)
        self._recompute_exec_button_state(btn)

    def _label_has_file(self, label):
        return getattr(label, 'chemin_complet', None) is not None

    def _recompute_exec_button_state(self, btn):
        missing = any(not self._label_has_file(l) for l in btn.required_labels)
        if missing:
            btn.configure(state=tk.DISABLED, bg=self.lighten_color(btn.base_color), fg=self.colors["muted_text"], activeforeground=self.colors["muted_text"], cursor="arrow")
        else:
            btn.configure(state=tk.NORMAL, bg=btn.base_color, fg=self.colors["text"], activeforeground=self.colors["text"], cursor="hand2")

    def _refresh_exec_buttons_for_label(self, label):
        for btn in self.exec_buttons:
            if label in getattr(btn, 'required_labels', []):
                self._recompute_exec_button_state(btn)

    def lighten_color(self, color):
        # Eclaircit la couleur pour l'état désactivé (mapping simple façon Tailwind 200)
        m = {
            self.colors["btn_exec1"]: "#bfdbfe",
            self.colors["btn_exec2"]: "#a7f3d0",
            self.colors["btn_exec3"]: "#ddd6fe",
            self.colors["btn_add"]: "#a7f3d0",
            self.colors["btn_del"]: "#fecaca",
        }
        return m.get(color, "#e5e7eb")

    def creer_case_fichier_moderne(self, titre, ajouter_fichier_callback, parent):
        # Cadre modernisé pour chaque cas fichier
        cadre = tk.Frame(parent, bg="#ffffff", padx=5, pady=5)
        cadre.pack(side=tk.LEFT, padx=15, pady=15, fill=tk.BOTH, expand=True)

        label = tk.Label(cadre, text=titre, font=(self.font_text[0], self.font_text[1], "bold") if platform.system() != "Darwin" else ("SF Pro Text", 12, "bold"), bg="#ffffff", fg=self.colors["text"]) 
        label.pack(fill=tk.X, anchor="w", pady=(0, 10))

        # Zone fichier avec design moderne (thème dépendant)
        file_zone_bg = "#2c2c2e" if self.is_dark else "#f8fafc"
        file_zone = tk.Frame(cadre, bg=file_zone_bg, bd=0, relief=tk.FLAT, padx=15, pady=12)
        file_zone.pack(fill=tk.X, pady=(0, 15))
        file_zone.configure(highlightbackground=self.colors["border"], highlightthickness=1)
        
        fichier_label = tk.Label(file_zone, text="Aucun fichier sélectionné", 
					       font=("SF Pro Text", 11) if platform.system() == "Darwin" else ("Segoe UI", 10), 
					       bg=file_zone_bg, fg=("#d1d5db" if self.is_dark else "#6b7280"),
					       anchor="w", padx=5, pady=5)
        fichier_label.pack(fill=tk.X)
        # Associer l'intitulé pour des messages d'erreur clairs
        fichier_label.display_name = titre

        # Boutons avec style moderne
        btn_frame = tk.Frame(cadre, bg="#ffffff")
        btn_frame.pack(fill=tk.X)
        
        ajouter_btn = tk.Button(btn_frame, text="📁 Sélectionner", 
                              command=lambda: ajouter_fichier_callback(fichier_label),
                              bg=self.colors["btn_add"], fg=self.colors["text"], 
                              font=(self.font_text[0], self.font_text[1]-1, "bold") if platform.system() != "Darwin" else ("SF Pro Text", 10, "bold"), relief=tk.FLAT, padx=12, pady=8, cursor="hand2", bd=0, activeforeground=self.colors["text"], activebackground=self.darken_color(self.colors["btn_add"]), takefocus=True, highlightthickness=2, highlightbackground=self.colors["border"], highlightcolor=self.colors["accent"]) 
        ajouter_btn.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        supprimer_btn = tk.Button(btn_frame, text="🗑️ Supprimer", 
                                command=lambda: self.supprimer_fichier_case(fichier_label),
                                bg=self.colors["btn_del"], fg=self.colors["text"], 
                                font=(self.font_text[0], self.font_text[1]-1, "bold") if platform.system() != "Darwin" else ("SF Pro Text", 10, "bold"), relief=tk.FLAT, padx=12, pady=8, cursor="hand2", bd=0, activeforeground=self.colors["text"], activebackground=self.darken_color(self.colors["btn_del"]), takefocus=True, highlightthickness=2, highlightbackground=self.colors["border"], highlightcolor=self.colors["accent"]) 
        supprimer_btn.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(8, 0))
        
        # Infobulles
        self._add_tooltip(ajouter_btn, "Choisir un fichier")
        self._add_tooltip(supprimer_btn, "Effacer le fichier sélectionné")

        # Ajouter des effets de survol pour les boutons
        def on_enter_add(e):
            ajouter_btn.configure(bg=self.darken_color(self.colors["btn_add"]))
        def on_leave_add(e):
            ajouter_btn.configure(bg=self.colors["btn_add"])
        def on_enter_del(e):
            supprimer_btn.configure(bg=self.darken_color(self.colors["btn_del"]))
        def on_leave_del(e):
            supprimer_btn.configure(bg=self.colors["btn_del"])
        
        ajouter_btn.bind("<Enter>", on_enter_add)
        ajouter_btn.bind("<Leave>", on_leave_add)
        supprimer_btn.bind("<Enter>", on_enter_del)
        supprimer_btn.bind("<Leave>", on_leave_del)

        return fichier_label

    def _add_tooltip(self, widget, text):
        Tooltip(widget, text)

    def ajouter_fichier1a(self, fichier_label):
        self.ajouter_fichier(fichier_label)

    def ajouter_fichier1b(self, fichier_label):
        self.ajouter_fichier(fichier_label)

    def ajouter_fichier2(self, fichier_label):
        self.ajouter_fichier(fichier_label)

    def ajouter_fichier3a(self, fichier_label):
        self.ajouter_fichier(fichier_label)

    def ajouter_fichier3b(self, fichier_label):
        self.ajouter_fichier(fichier_label)

    def ajouter_fichier(self, fichier_label):
        fichier = filedialog.askopenfilename(
            title="Sélectionner un fichier",
            initialdir=self.last_dir,
            filetypes=[
                ("Fichiers CSV", "*.csv"),
                ("Fichiers Excel", "*.xlsx;*.xls"),
                ("Tous les fichiers", "*.*")
            ]
        )
        if fichier:
            nom = os.path.basename(fichier)
            taille = os.path.getsize(fichier) // 1024  # Taille en Ko
            fichier_label.config(
                text=f"✅ {nom} ({taille} Ko)", 
                fg="#065f46",  # Vert foncé pour contraste sur fond vert clair
                bg="#ecfdf5"   # Fond vert très clair pour indiquer qu'un fichier est sélectionné
            )
            # Stocker le chemin complet comme attribut du label
            fichier_label.chemin_complet = fichier
            # Mémoriser le répertoire pour la prochaine ouverture
            self.last_dir = os.path.dirname(fichier)
            # Style sélection selon thème
            if self.is_dark:
                fichier_label.config(fg="#a7f3d0", bg=fichier_label.master.cget("bg"))
            else:
                fichier_label.config(fg="#065f46", bg="#ecfdf5")
            self._refresh_exec_buttons_for_label(fichier_label)

    def supprimer_fichier_case(self, fichier_label):
        # Réinitialiser style selon thème
        bg = fichier_label.master.cget("bg")
        fg = "#d1d5db" if self.is_dark else "#6b7280"
        fichier_label.config(text="Aucun fichier sélectionné", fg=fg, bg=bg)
        if hasattr(fichier_label, 'chemin_complet'):
            delattr(fichier_label, 'chemin_complet')
        self._refresh_exec_buttons_for_label(fichier_label)
        # Supprimer le chemin complet si présent
        if hasattr(fichier_label, 'chemin_complet'):
            delattr(fichier_label, 'chemin_complet')

    def executer_script1(self):
        self.executer_script("process_JV", self.case_fichier1a, self.case_fichier1b)

    def executer_script2(self):
        self.executer_script("process_JC", self.case_fichier2)

    def executer_script3(self):
        self.executer_script("process_JB", self.case_fichier3a, self.case_fichier3b)

    def executer_script(self, script_name, *fichiers_labels_or_lists):
        fichiers = []
        output_filename = None
        
        # Valider que tous les fichiers obligatoires sont sélectionnés
        fichiers_obligatoires_manquants = []
        
        # Extract output filename if it's a string at the end
        for item in fichiers_labels_or_lists:
            if isinstance(item, str):  # Si c'est une chaîne de caractères (un nom de sortie)
                output_filename = item
                # Don't continue here, we need to keep the output filename for "jb"
                if script_name != "jb":
                    continue
            elif isinstance(item, tk.Listbox):
                fichiers.extend(item.get(0, tk.END))
            elif isinstance(item, tk.Label):
                if item.cget("text") != "Aucun fichier sélectionné":
                    # Utiliser le chemin complet du fichier s'il est disponible
                    if hasattr(item, 'chemin_complet'):
                        fichiers.append(item.chemin_complet)
                    else:
                        # Fallback au comportement précédent si le chemin n'est pas disponible
                        file_text = item.cget("text").split(" (")[0].replace("✅ ", "")
                        fichiers.append(file_text)
                else:
                    # Ajouter l'intitulé exact si disponible
                    manquant = getattr(item, 'display_name', "un fichier obligatoire")
                    fichiers_obligatoires_manquants.append(manquant)

        # Vérifier si des fichiers obligatoires sont manquants
        if fichiers_obligatoires_manquants:
            liste = "\n - " + "\n - ".join(fichiers_obligatoires_manquants)
            messagebox.showwarning(
                "Fichiers manquants", 
                f"Veuillez sélectionner tous les fichiers obligatoires pour {script_name.replace('process_', '').upper()} :{liste}"
            )
            return

        if not fichiers:
            messagebox.showwarning("Aucun fichier", f"Veuillez d'abord sélectionner des fichiers pour {script_name}.")
            return

        # Add the output filename to fichiers for "jb" script if it's not already there
        if script_name == "jb" and output_filename and output_filename not in fichiers:
            fichiers.append(output_filename)

        try:
            # Obtenir le chemin absolu du répertoire de l'application
            app_dir = os.path.dirname(os.path.abspath(__file__))
            
            # Construire le chemin absolu de l'exécutable
            if platform.system() == "Windows":
                # Pour Windows, chercher dans le même répertoire que le script
                exe_path = os.path.join(app_dir, f"{script_name}.exe")
            else:
                # Pour Linux/Mac, chercher dans le même répertoire que le script
                exe_path = os.path.join(app_dir, script_name)
            
            # Vérifier que l'exécutable existe
            if not os.path.exists(exe_path):
                # Essayer de chercher dans le répertoire parent
                parent_dir = os.path.dirname(app_dir)
                if platform.system() == "Windows":
                    exe_path = os.path.join(parent_dir, f"{script_name}.exe")
                else:
                    exe_path = os.path.join(parent_dir, script_name)
                
                if not os.path.exists(exe_path):
                    messagebox.showerror("Erreur", f"L'exécutable '{script_name}' n'a pas été trouvé.\nChemin cherché: {exe_path}")
                    return
            
            # Afficher les informations de débogage
            print(f"Chemin de l'exécutable: {exe_path}")
            print(f"Fichiers à traiter:")
            for f in fichiers:
                print(f"  - {f}")
                if not os.path.exists(f):
                    print(f"    ATTENTION: Ce fichier n'existe pas!")
            
            # Préparation des arguments pour subprocess
            args = [exe_path] + fichiers
            print(f"Commande complète: {' '.join(args)}")
            
            # Exécuter le processus avec les chemins absolus
            result = subprocess.run(args, capture_output=True, text=True, cwd=os.path.dirname(exe_path))

            if result.returncode == 0:
                # Messages de succès personnalisés selon le script
                success_messages = {
                    "process_JV": "✅ Journal Vente/Encaissement généré avec succès !",
                    "process_JC": "✅ Journal Caisse généré avec succès !",
                    "process_JB": "✅ Journal Bancaire généré avec succès !"
                }
                message = success_messages.get(script_name, f"✅ Le {script_name} a été généré avec succès.")
                messagebox.showinfo("Génération réussie", message)
                print(f"Sortie standard: {result.stdout}")
            else:
                error_msg = f"❌ Erreur lors de la génération du journal {script_name.replace('process_', '').upper()}:\n\n"
                if result.stderr:
                    error_msg += result.stderr
                else:
                    error_msg += "Aucun message d'erreur spécifique n'a été retourné.\n"
                    error_msg += f"Sortie standard: {result.stdout}"
                messagebox.showerror("Erreur de génération", error_msg)
                print(f"Code de retour: {result.returncode}")
                print(f"Erreur: {result.stderr}")
                print(f"Sortie: {result.stdout}")
        except Exception as e:
            messagebox.showerror("Erreur système", f"❌ Une erreur s'est produite lors de l'exécution de {script_name}:\n\n{str(e)}")
            import traceback
            print(traceback.format_exc())


if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()
