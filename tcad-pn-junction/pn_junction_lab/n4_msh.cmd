Title "Untitled"

Controls {
}

IOControls {
	EnableSections
}

Definitions {
	Constant "p-doping-profile" {
		Species = "BoronActiveConcentration"
		Value = 1e+17
	}
	Constant "n-doping-profile" {
		Species = "PhosphorusActiveConcentration"
		Value = 1e+17
	}
	Refinement "domain-ref-size" {
		MaxElementSize = ( 1 0.1 )
		MinElementSize = ( 0.01 0.01 )
	}
	Refinement "junction-ref-size" {
		MaxElementSize = ( 0.1 0.01 )
		MinElementSize = ( 0.001 0.001 )
	}
}

Placements {
	Constant "p-doping-placement" {
		Reference = "p-doping-profile"
		EvaluateWindow {
			Element = region ["p-region"]
		}
	}
	Constant "n-doping-placement" {
		Reference = "n-doping-profile"
		EvaluateWindow {
			Element = region ["n-region"]
		}
	}
	Refinement "domain-ref-pl" {
		Reference = "domain-ref-size"
		RefineWindow = Rectangle [(0 0) (0.5 1)]
	}
	Refinement "junction-ref-pl" {
		Reference = "junction-ref-size"
		RefineWindow = Rectangle [(0 0.42) (0.5 0.58)]
	}
}

