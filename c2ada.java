import java.applet.*;
import java.awt.*;

public class c2ada extends NoFlickerApplet {
// public class c2ada extends Applet {

    final int rectSize = 100;
    Image adaI, cI;
    Color cBack, adaBack;

    public void init() {
	add(new Button("translate"));
	adaI    = getImage(getCodeBase(), "ada.gif");
	cI      = getImage(getCodeBase(), "c.gif");
	adaBack = cBack = new Color(255, 255, 255);
	repaint();
    }

    public void paint (Graphics g) {
	g.setColor(cBack);
	g.fillRect(0, 0, rectSize, rectSize);
	g.drawImage(cI, 0, 0, this);

	g.setColor(adaBack);
	g.fillRect(rectSize, 0, rectSize, rectSize);
	g.drawImage(adaI, rectSize, 0, this);

	// g.setColor(Color.black);	// a vertical line between the 2
	// g.drawLine(rectSize, 0, rectSize, rectSize);
	// System.out.println("painting " +cBack +adaBack);
    }

    public void doit (Graphics g) {
	int adaBackR, adaBackG, adaBackB, cBackR, cBackG, cBackB;

	adaBackR = adaBackG = adaBackB = 0;

	while (adaBackB < 255) {
	    cBackR  = 255-adaBackR;
	    cBackG  = 255-adaBackG;
	    cBackB  = 255-adaBackB;
	    adaBack = new Color(adaBackR, adaBackG, adaBackB);
	    cBack   = new Color(cBackR, cBackG, cBackB);
	    update(g);
	    if (adaBackR < 255)
		adaBackR += 8;
	    else if (adaBackG < 255)
		adaBackG += 8;
	    else if (adaBackB < 255)
		adaBackB += 8;
	    if (adaBackR > 255) adaBackR = 255;
	    if (adaBackG > 255) adaBackG = 255;
	    if (adaBackB > 255) adaBackB = 255;
	}
    }

    public boolean action (Event evt, Object arg) {
	if (evt.target instanceof Button) {
	    String label = (String) arg;
	    if (label.equals("translate")) {
		// System.out.println("repainting");
		doit(getGraphics());
		return true;
	    }
	}
	return false;
    }
}

/**
 *
 *  Extension to base applet class to avoid display flicker
 *
 * @version             1.0, 27 Oct 1995
 * @author Matthew Gray
 *
 * Copyright (C) 1996 by Matthew Gray
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of Matthew Gray or net.Genesis not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  I make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

// public class NoFlickerApplet extends Applet {
class NoFlickerApplet extends Applet {
  private Image offScreenImage;
  private Graphics offScreenGraphics;
  private Dimension offScreenSize;

  public final synchronized void update (Graphics theG) {
      Dimension d = size();
      if((offScreenImage == null) || (d.width != offScreenSize.width) ||
         (d.height != offScreenSize.height)) {
	  // System.out.println("update init");
          offScreenImage = createImage(d.width, d.height);
          offScreenSize = d;
          offScreenGraphics = offScreenImage.getGraphics();
          offScreenGraphics.setFont(getFont());
      }
      // System.out.println("update");
      offScreenGraphics.fillRect(0,0,d.width, d.height);
      paint(offScreenGraphics);
      theG.drawImage(offScreenImage, 0, 0, null);
  }
}
