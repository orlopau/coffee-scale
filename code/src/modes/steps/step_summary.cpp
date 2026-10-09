#include "step_summary.h"
#include "ui/widgets.h"

// size of the QR code of a recipe's URL
#define QR_SIZE 54

RecipeSummaryStep::RecipeSummaryStep(RecipeStepState &state) : state(state) {}

void RecipeSummaryStep::render(Canvas &canvas)
{
    const Recipe *recipe = state.originalRecipe;
    const int width = canvas.width();

    if (recipe->url[0] == '\0')
    {
        int y = Widgets::titleLine(canvas, recipe->name);
        Widgets::textWrapped(canvas, recipe->note, y + 2, 0, width);
    }
    else
    {
        Widgets::textWrapped(canvas, recipe->note, 0, 1, width - QR_SIZE - 3);
        Widgets::qrCode(canvas, recipe->url, width - QR_SIZE, (canvas.height() - QR_SIZE) / 2);
    }
}
