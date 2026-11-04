; *** IDEAL PN JUNCTION - sde_dvs.cmd ***
; *** INITIALIZATION ***

(sde:clear)
(sdegeo:set-default-boolean "ABA")

; *** DEFINITIONS ***
(define material "@material@")
(define L 0.5)
(define Wp 0.5)
(define Wn 0.5)
(define p_doping @p_doping@)
(define n_doping @n_doping@)
(define xmax 1)
(define xmin 0.01)
(define ymax 0.1)
(define ymin 0.01)

; *** GEOMETRY ***
; convention: x=length y=thickness
; create p region
(sdegeo:create-rectangle (position 0 0 0) (position L (- Wp 0.02) 0) material "l-region")
(sdegeo:create-rectangle (position 0 (- Wp 0.02) 0) (position L Wp 0) material "p-region")
; create n region
(sdegeo:create-rectangle (position 0 Wp 0) (position L (+ Wp 0.02) 0) material "n-region")
(sdegeo:create-rectangle (position 0 (+ Wp 0.02) 0) (position L (+ Wn Wp) 0) material "r-region")

; *** CONTACTS ***
; a) SET VERTEXES
; 1st vertex on p_contact
(sdegeo:insert-vertex (position 0 0 0))
; 2nd vertex on p_contact
(sdegeo:insert-vertex (position L 0 0))
; 1st vertex on n_contact
(sdegeo:insert-vertex (position 0 (+ Wn Wp) 0))
; 2nd vertex on n_contact
(sdegeo:insert-vertex (position L (+ Wn Wp) 0))
; b) SET EDGE (DECLARATION, ACTIVATION AND DEFINITION)
; p_contact
(sdegeo:define-contact-set "p_contact" 4 (color:rgb 1 0 0) "##")
(sdegeo:set-current-contact-set "p_contact")
(sdegeo:define-2d-contact (find-edge-id (position (* L 0.5) 0 0)) "p_contact")
; n_contact
(sdegeo:define-contact-set "n_contact" 4 (color:rgb 1 0 0) "##")
(sdegeo:set-current-contact-set "n_contact")
(sdegeo:define-2d-contact (find-edge-id (position (* L 0.5) (+ Wn Wp) 0)) "n_contact")

; *** DOPING ***
; p-region
(sdedr:define-constant-profile "p-doping-profile" "BoronActiveConcentration" @p_doping@)
(sdedr:define-constant-profile-region "p-doping-placement" "p-doping-profile" "p-region")
(sdedr:define-constant-profile-placement "p-doping-placement" "p-doping-profile" "p-doping-window")
; n-region
(sdedr:define-constant-profile "n-doping-profile" "PhosphorusActiveConcentration" @n_doping@)
(sdedr:define-constant-profile-region "n-doping-placement" "n-doping-profile" "n-region")
(sdedr:define-constant-profile-placement "n-doping-placement" "n-doping-profile" "n-doping-window")

(sdedr:define-constant-profile-region "l-doping-placement" "p-doping-profile" "l-region")
(sdedr:define-constant-profile-placement "l-doping-placement" "p-doping-profile" "l-doping-window")

(sdedr:define-constant-profile-region "r-doping-placement" "n-doping-profile" "r-region")
(sdedr:define-constant-profile-placement "r-doping-placement" "n-doping-profile" "r-doping-window")

; *** MESH ***
; * WHOLE DOMAIN
(sdedr:define-refeval-window "domain-ref" "Rectangle" (position 0 0 0) (position L (+ Wn Wp 0.96) 0))
(sdedr:define-refinement-size "domain-ref-size" xmax ymax xmin ymin)
(sdedr:define-refinement-placement "domain-ref-pl" "domain-ref-size" "domain-ref")
; * p-n JUNCTION REFINEMENT
(sdedr:define-refinement-window "junction-ref" "Rectangle" (position 0 (- L 0.08) 0) (position L (+ L 0.08) 0))
(sdedr:define-refinement-size "junction-ref-size" (/ xmax 10) (/ ymax 10) (/  xmin 10) (/ ymin 10))
(sdedr:define-refinement-placement "junction-ref-pl" "junction-ref-size" "junction-ref")

; * BUILDING MESH
(sde:build-mesh "snmesh" "-a -c boxmethod" "n@node@")
