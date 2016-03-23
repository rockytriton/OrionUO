/****************************************************************************
**
** JournalGump.cpp
**
** Copyright (C) September 2015 Hotride
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
*****************************************************************************
*/
//----------------------------------------------------------------------------
#include "stdafx.h"
//----------------------------------------------------------------------------
TGumpJournal::TGumpJournal(DWORD serial, short x, short y, bool minimized)
: TGump(GT_JOURNAL, serial, x, y), m_Height(200), m_HeightBuffer(0),
m_LastScrollChangeTime(0)
{
	if (minimized)
	{
		m_Minimized = true;
		m_MinimizedX = x;
		m_MinimizedY = y;
	}
}
//----------------------------------------------------------------------------
TGumpJournal::~TGumpJournal()
{
}
//----------------------------------------------------------------------------
void TGumpJournal::PrepareTextures()
{
	UO->ExecuteGump(0x001F);
	UO->ExecuteGumpPart(0x00D2, 2);
	UO->ExecuteGumpPart(0x0820, 6);
	UO->ExecuteGumpPart(0x082A, 7);
}
//----------------------------------------------------------------------------
void TGumpJournal::SetHeight(int val)
{
	if (val < 120)
		val = 120;

	int buf = (GetSystemMetrics(SM_CYSCREEN) - 50);

	if (val >= buf)
		val = buf;

	m_Height = val;
}
//----------------------------------------------------------------------------
void TGumpJournal::ChangeHeight()
{
	if (m_HeightBuffer)
		SetHeight(m_Height + (g_MouseY - m_HeightBuffer));

	m_HeightBuffer = 0;
	g_ResizedGump = NULL;
}
//---------------------------------------------------------------------------
void TGumpJournal::GenerateFrame(int posX, int posY)
{
	if (!g_DrawMode)
	{
		m_FrameRedraw = false;
		m_FrameCreated = false;

		return;
	}

	CalculateGumpState();

	glNewList((GLuint)this, GL_COMPILE);

		if (m_Minimized)
		{
			UO->DrawGump(0x0830, 0, 0, 0);

			glEndList();

			m_FrameRedraw = true;
			m_FrameCreated = true;

			return;
		}

		int height = m_Height;

		if (m_HeightBuffer)
		{
			height += (g_MouseY - m_HeightBuffer);

			if (height < 120)
				height = 120;

			int buf = (GetSystemMetrics(SM_CYSCREEN) - 50);

			if (height >= buf)
				height = buf;
		}

		UO->DrawGump(0x082D, 0, 137, 0); //Minimize
		UO->DrawGump(0x0820, 0, 0, 23); //Top scroll
		UO->DrawGump(0x082A, 0, 111, 34); //Journal text gump
		
		//Journal body
		int curposY = 59;
		
		while (true)
		{
			int deltaHeight = height - (curposY - 36);

			if (deltaHeight  < 70)
			{
				if (deltaHeight > 0)
					UO->DrawGump(0x0821, 0, 18, curposY, 0, deltaHeight);

				break;
			}
			else
				UO->DrawGump(0x0821, 0, 18, curposY);

			curposY += 70;

			deltaHeight = height - (curposY - 36);

			if (deltaHeight < 70)
			{
				if (deltaHeight > 0)
					UO->DrawGump(0x0822, 0, 18, curposY, 0, deltaHeight);

				break;
			}
			else
				UO->DrawGump(0x0822, 0, 18, curposY);

			curposY += 70;
		}
		
		//Up
		//UO->DrawGump(0x001F, 0, posX + 257, posY + 72); //Scroller
		//Down
		//UO->DrawGump(0x001F, 0, posX + 257, posY + Height - 17); //Scroller
		
		int scrollerY = height - 17;

		if (g_GumpPressedScroller && g_LastObjectLeftMouseDown == ID_GJ_SCROLLER) //Scroller pressed
		{
			int currentY = (g_MouseY - 10) - ((int)g_GumpTranslateY + 72); //Scroller position

			if (currentY < (scrollerY - 72))
			{
				if (currentY > 0)
				{
					scrollerY  -= 72;

					float ValPer = (currentY / (float)scrollerY) * 100.0f;

					int jsize = Journal->Size;

					jsize = jsize - (int)((jsize * ValPer) / 100.0f);
					if (jsize < 1)
						jsize = 1;
					else if (jsize > Journal->Size)
						jsize = Journal->Size;

					Journal->SelectionIndex = jsize;

					scrollerY = currentY + 72;
				}
				else
				{
					scrollerY = 72;
					Journal->SelectionIndex = Journal->Size;
				}
			}
			else
				Journal->SelectionIndex = 1;
		}
		else if (Journal->SelectionIndex)
		{
			int jsize = Journal->Size;

			float ValPer = (Journal->SelectionIndex / (float)jsize) * 100.0f;

			jsize = (int)((jsize * ValPer) / 100.0f);
			if (jsize < 1)
				jsize = 1;
			else if (jsize > Journal->Size)
				jsize = Journal->Size;
			
			if (jsize == Journal->Size)
				scrollerY = 72;
			else if (jsize != 1)
			{
				scrollerY -= 72;
				scrollerY = (scrollerY + 72) - (int)((scrollerY * ValPer) / 100.0f);
			}
		}
		
		UO->DrawGump(0x001F, 0, 257, scrollerY); //Scroller
		
		UO->DrawGump(0x082B, 0, 30, 58); //Top line
		UO->DrawGump(0x0824, 0, 249, 56); //^ button
		UO->DrawGump(0x082B, 0, 31, height + 21); //Bottom line
		UO->DrawGump(0x0825, 0, 249, height + 17); //v button
		
		if (Journal->SelectionIndex)
			UO->DrawGump(0x082C, 0, 233, height + 17); //Lock
		
		UO->DrawGump(0x0823, 0, 18, height + 34); //Bottom scroll
		
		UO->DrawGump(0x00D2 + (int)g_JournalShowSystem, 0, 40, height + 43); //Show System
		FontManager->DrawA(1, "System", 0x0386, 63, height + 47);
		
		UO->DrawGump(0x00D2 + (int)g_JournalShowObjects, 0, 126, height + 43); //Show Objects
		FontManager->DrawA(1, "Objects", 0x0386, 149, height + 47);
		
		UO->DrawGump(0x00D2 + (int)g_JournalShowClient, 0, 210, height + 43); //Show Client
		FontManager->DrawA(1, "Client", 0x0386, 233, height + 47);
		
		WORD gumpID = 0x082E + (int)(g_GumpSelectElement == ID_GJ_BUTTON_RESIZE);
		UO->DrawGump(gumpID, 0, 137, height + 66); //Resize


		g_GL.Sicceor((int)g_GumpTranslateX + 38, (int)g_GumpTranslateY + 70, 214, height - 50);

		//��������� ��������� ������ ����
		int textOffsY = height + 16;
		int textYBounds = 70;
		int drawX = 38;

		int journalHeight = 0;

		if (g_JournalShowSystem)
			journalHeight += Journal->SystemHeight;

		if (g_JournalShowObjects)
			journalHeight += Journal->ObjectsHeight;

		if (g_JournalShowClient)
			journalHeight += Journal->ClientHeight;

		if (journalHeight < height - 50)
		{
			textYBounds = height;
			textOffsY = 70;

			TTextData *td = Journal->m_Top;

			while (td != NULL)
			{
				//if (TextOffsY > TextYBounds) break;

				if (td->CanBeDrawedInJournalGump())
				{
					TTextTexture &tth = td->m_Texture;

					tth.Draw(drawX, textOffsY);
					textOffsY += tth.Height;
				}

				td = td->m_Next;
			}
		}
		else
		{
			if (Journal->SelectionIndex)
			{
				if (Journal->SelectionIndex >= Journal->Size)
				{
					textYBounds = height;
					textOffsY = 70;

					TTextData *td = Journal->m_Top;

					while (td != NULL)
					{
						if (textOffsY > textYBounds)
							break;

						if (td->CanBeDrawedInJournalGump())
						{
							TTextTexture &tth = td->m_Texture;

							tth.Draw(drawX, textOffsY);
							textOffsY += tth.Height;
						}

						td = td->m_Next;
					}
				}
				else
				{
					int testHeight = 0;
					int maxID = Journal->SelectionIndex;
					int currentID = 0;

					TTextData *td = Journal->m_Head;
					TTextData *last_td = Journal->m_Head;

					while (td != NULL)
					{
						currentID++;

						if (td->CanBeDrawedInJournalGump())
						{
							TTextTexture &tth = td->m_Texture;
							testHeight += tth.Height;

							last_td = td;
						}

						if (currentID >= maxID)
							break;

						td = td->m_Prev;
					}

					textYBounds = height;
					textOffsY = 70;

					td = last_td;

					while (td != NULL)
					{
						if (textOffsY > textYBounds)
							break;

						if (td->CanBeDrawedInJournalGump())
						{
							TTextTexture &tth = td->m_Texture;

							tth.Draw(drawX, textOffsY);
							textOffsY += tth.Height;
						}

						td = td->m_Next;
					}
				}
			}
			else
			{
				TTextData *td = Journal->m_Head;

				while (td != NULL)
				{
					if (textOffsY < textYBounds)
						break;

					if (td->CanBeDrawedInJournalGump())
					{
						TTextTexture &tth = td->m_Texture;

						textOffsY -= tth.Height;
						tth.Draw(drawX, textOffsY);
					}

					td = td->m_Prev;
				}
			}
		}

		glDisable(GL_SCISSOR_TEST);

	glEndList();

	m_FrameRedraw = true;
	m_FrameCreated = true;
}
//----------------------------------------------------------------------------
int TGumpJournal::Draw(bool &mode)
{
	DWORD index = (DWORD)this;

	CalculateGumpState();

	if (Minimized)
		m_HeightBuffer = 0;
	
	int height = m_Height;

	bool needUpdateFrame = false;

	if (m_HeightBuffer)
	{
		height += (g_MouseY - m_HeightBuffer);

		if (height < 120)
			height = 120;

		int buf = (GetSystemMetrics(SM_CYSCREEN) - 50);

		if (height >= buf)
			height = buf;

		if (mode)
			needUpdateFrame = true;
	}

	DWORD ticks = GetTickCount();

	if (g_GumpPressedScroller && m_LastScrollChangeTime < ticks)
	{
		if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_UP)
		{
			if (Journal->SelectionIndex < Journal->Size)
				Journal->IncSelectionIndex();
			else
				Journal->SelectionIndex = Journal->Size;
		}
		else if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_DOWN)
		{
			if (Journal->SelectionIndex > 1)
				Journal->DecSelectionIndex();
			else
				Journal->SelectionIndex = 1;
		}

		m_LastScrollChangeTime = ticks + SCROLL_LISTING_DELAY;
	}
	
	int scrollerY = height - 17;

	if (!m_Minimized) //Check scroller
	{
		if (g_GumpPressedScroller && g_LastObjectLeftMouseDown == ID_GJ_SCROLLER) //Scroller pressed
		{
			int currentY = (g_MouseY - 10) - ((int)g_GumpTranslateY + 72); //Scroller position

			if (currentY < (scrollerY - 72))
			{
				if (currentY > 0)
				{
					scrollerY  -= 72;
					float ValPer = (currentY / (float)scrollerY) * 100.0f;

					int jsize = Journal->Size;

					jsize = jsize - (int)((jsize * ValPer) / 100.0f);
					if (jsize < 1)
						jsize = 1;
					else if (jsize > Journal->Size)
						jsize = Journal->Size;

					Journal->SelectionIndex = jsize;

					scrollerY = currentY + 72;
				}
				else
				{
					scrollerY = 72;
					Journal->SelectionIndex = Journal->Size;
				}
			}
			else
				Journal->SelectionIndex = 1;

			if (mode)
				needUpdateFrame = true;
		}
		else if (Journal->SelectionIndex)
		{
			int jsize = Journal->Size;

			float ValPer = (Journal->SelectionIndex / (float)jsize) * 100.0f;

			jsize = (int)((jsize * ValPer) / 100.0f);
			if (jsize < 1)
				jsize = 1;
			else if (jsize > Journal->Size)
				jsize = Journal->Size;
			
			if (jsize == Journal->Size)
				scrollerY = 72;
			else if (jsize != 1)
			{
				scrollerY -= 72;
				scrollerY = (scrollerY + 72) - (int)((scrollerY * ValPer) / 100.0f);
			}

			if (mode)
				needUpdateFrame = true;
		}
	}

	if (mode)
	{
		if (needUpdateFrame || !m_FrameCreated || (g_GumpSelectElement && !m_HeightBuffer))
			GenerateFrame(0, 0);
		else if (m_FrameRedraw)
		{
			GenerateFrame(0, 0);
			FrameRedraw = false;
		}

		glTranslatef(g_GumpTranslateX, g_GumpTranslateY, 0.0f);

		glCallList((GLuint)index);

		if (m_Minimized)
		{
			glTranslatef(-g_GumpTranslateX, -g_GumpTranslateY, 0.0f);

			return 0;
		}

		int lx = 0;
		DrawLocker(lx, lx);

		glTranslatef(-g_GumpTranslateX, -g_GumpTranslateY, 0.0f);
	}
	else
	{
		int oldMouseX = g_MouseX;
		int oldMouseY = g_MouseY;
		g_MouseX -= (int)g_GumpTranslateX;
		g_MouseY -= (int)g_GumpTranslateY;

		if (m_Minimized)
		{
			if (UO->GumpPixelsInXY(0x0830, 0, 0))
			{
				g_LastSelectedObject = 0;
				g_LastSelectedGump = index;
			}

			return 0;
		}
		
		int LSG = 0;
		
		if (UO->GumpPixelsInXY(0x082D, 137, 0)) //Minimize
		{
			LSG = ID_GJ_BUTTON_MINIMIZE;
			g_LastSelectedGump = index;
		}
		
		if (UO->GumpPixelsInXY(0x0820, 0, 23)) //Top scroll
		{
			g_LastSelectedObject = 0;
			g_LastSelectedGump = index;
		}

		//Journal body
		int curposY = 59;

		while (true)
		{
			int deltaHeight = height - (curposY - 36);

			if (deltaHeight  < 70)
			{
				if (deltaHeight > 0)
				{
					if (UO->GumpPixelsInXY(0x0821, 18, curposY, 0, deltaHeight))
					{
						g_LastSelectedObject = 0;
						g_LastSelectedGump = index;
					}
				}

				break;
			}
			else if (UO->GumpPixelsInXY(0x0821, 18, curposY))
			{
				g_LastSelectedObject = 0;
				g_LastSelectedGump = index;

				break;
			}

			curposY += 70;

			deltaHeight = height - (curposY - 36);

			if (deltaHeight < 70)
			{
				if (deltaHeight > 0)
				{
					if (UO->GumpPixelsInXY(0x0822, 18, curposY, 0, deltaHeight))
					{
						g_LastSelectedObject = 0;
						g_LastSelectedGump = index;
					}
				}

				break;
			}
			else if (UO->GumpPixelsInXY(0x0822, 18, curposY))
			{
				g_LastSelectedObject = 0;
				g_LastSelectedGump = index;

				break;
			}

			curposY += 70;
		}

		if (UO->GumpPixelsInXY(0x0823, 18, height + 34)) //Bottom scroll
		{
			g_LastSelectedObject = 0;
			g_LastSelectedGump = index;
		}

		if (g_LastSelectedGump == index)
		{
			if (UO->GumpPixelsInXY(0x00D2, 40, height + 43)) //Show System
				LSG = ID_GJ_SHOW_SYSTEM;
			else if (UO->GumpPixelsInXY(0x00D2, 126, height + 43)) //Show Objects
				LSG = ID_GJ_SHOW_OBJECTS;
			else if (UO->GumpPixelsInXY(0x00D2, 210, height + 43)) //Show Client
				LSG = ID_GJ_SHOW_CLIENTS;
			else if (UO->GumpPixelsInXY(0x082C, 233, height + 17)) //Lock
				LSG = ID_GJ_BUTTON_LOCK;
			else if (UO->GumpPixelsInXY(0x0824, 249, 56)) //^ button
				LSG = ID_GJ_BUTTON_UP;
			else if (UO->GumpPixelsInXY(0x0825, 249, height + 17)) //v button
				LSG = ID_GJ_BUTTON_DOWN;
		}
		
		if (UO->GumpPixelsInXY(0x001F, 257, scrollerY)) //Scroller
		{
			g_LastSelectedGump = index;
			LSG = ID_GJ_SCROLLER;
		}
		
		if (UO->GumpPixelsInXY(0x082E, 137, height + 66)) //Resize
		{
			g_LastSelectedGump = index;
			LSG = ID_GJ_BUTTON_RESIZE;
		}

		if (LSG != 0)
			g_LastSelectedObject = LSG;

		if (g_ShowGumpLocker && UO->PolygonePixelsInXY(0, 0, 10, 14))
		{
			g_LastSelectedObject = ID_GJ_LOCK_MOVING;
			g_LastSelectedGump = index;
		}

		g_MouseX = oldMouseX;
		g_MouseY = oldMouseY;

		return LSG;
	}

	return 0;
}
//----------------------------------------------------------------------------
void TGumpJournal::OnLeftMouseUp()
{
	if (g_LastObjectLeftMouseDown != g_LastSelectedObject)
		return;

	DWORD ticks = GetTickCount();

	if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_MINIMIZE)
	{
		m_Minimized = true;
		GenerateFrame(m_MinimizedX, m_MinimizedY);
	}
	else if (g_LastObjectLeftMouseDown == ID_GJ_LOCK_MOVING)
	{
		m_LockMoving = !m_LockMoving;
		g_CancelDoubleClick = true;
	}
	else if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_RESIZE)
	{
		if (!m_HeightBuffer)
		{
			m_HeightBuffer = g_DroppedLeftMouseY;
			g_ResizedGump = this;
		}
		else
		{
			SetHeight(m_Height + (g_MouseY - m_HeightBuffer));
			m_HeightBuffer = 0;
			g_ResizedGump = NULL;
		}
	}
	else if (g_LastObjectLeftMouseDown == ID_GJ_SHOW_SYSTEM)
		g_JournalShowSystem = !g_JournalShowSystem;
	else if (g_LastObjectLeftMouseDown == ID_GJ_SHOW_OBJECTS)
		g_JournalShowObjects = !g_JournalShowObjects;
	else if (g_LastObjectLeftMouseDown == ID_GJ_SHOW_CLIENTS)
		g_JournalShowClient = !g_JournalShowClient;
	else if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_LOCK)
	{
		Journal->SelectionIndex = 0;
		UpdateFrame();
	}
	else if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_UP && m_LastScrollChangeTime < ticks)
	{
		if (Journal->SelectionIndex < Journal->Size)
			Journal->IncSelectionIndex();
		else
			Journal->SelectionIndex = Journal->Size;

		m_LastScrollChangeTime = ticks + SCROLL_LISTING_DELAY;
		UpdateFrame();
	}
	else if (g_LastObjectLeftMouseDown == ID_GJ_BUTTON_DOWN && m_LastScrollChangeTime < ticks)
	{
		if (Journal->SelectionIndex > 1)
			Journal->DecSelectionIndex();
		else
			Journal->SelectionIndex = 1;

		m_LastScrollChangeTime = ticks + SCROLL_LISTING_DELAY;
		UpdateFrame();
	}
}
//----------------------------------------------------------------------------
bool TGumpJournal::OnLeftMouseDoubleClick()
{
	if (m_Minimized)
	{
		m_Minimized = false;
		GenerateFrame(m_X, m_Y);

		return true;
	}

	return false;
}
//----------------------------------------------------------------------------
void TGumpJournal::OnMouseWheel(MOUSE_WHEEL_STATE &state)
{
	if (!m_Minimized && !g_LeftMouseDown && !g_RightMouseDown)
	{
		DWORD ticks = GetTickCount();

		if (state == MWS_UP && m_LastScrollChangeTime < ticks)
		{
			if (Journal->SelectionIndex < Journal->Size)
				Journal->IncSelectionIndex();
			else
				Journal->SelectionIndex = Journal->Size;

			m_LastScrollChangeTime = ticks + (SCROLL_LISTING_DELAY / 4);
			UpdateFrame();
		}
		else if (state == MWS_DOWN && m_LastScrollChangeTime < ticks)
		{
			if (Journal->SelectionIndex > 1)
				Journal->DecSelectionIndex();
			else
				Journal->SelectionIndex = 1;

			m_LastScrollChangeTime = ticks + (SCROLL_LISTING_DELAY / 4);
			UpdateFrame();
		}
	}
}
//----------------------------------------------------------------------------